import time

from nicegui import app, ui

import angles
import clearsky
import compass
import estimate
import history
import link
import sideview
import status
import sun
import telemetry
import topview
import power

KEYS = ('MODE', 'ST', 'FLT', 'ERR', 'IMU', 'MOV', 'SUN', 'SEL', 'PAN', 'HDG', 'AZ', 'EL', 'A0', 'A1')
CURRENT_STYLE = 'font-mono text-sm'
OLD_STYLE = 'font-mono text-sm text-gray-400'
FAULT_STYLE = 'font-bold text-red-700'
FAULT_HELP_STYLE = 'text-red-700'
OLD_TEXT_STYLE = 'text-gray-400'
PANEL_STYLE = 'text-xl font-mono'
NOTE_STYLE = 'text-sm text-gray-500'
GRAPH_STYLE = 'grow basis-0 min-w-0 h-60'
VOLTS_TOP = 15.0
AMPS_TOP = 1.5
WATTS_TOP = 15.0
VOLTS_COLOUR = '#2a78d6'
AMPS_COLOUR = '#c2410c'
WATTS_COLOUR = '#15803d'
MODEL_COLOUR = '#9ca3af'
ENERGY_COLOUR = '#7c3aed'
DAY_GRAPH_STYLE = 'grow min-w-0 h-72'
DAY_WATTS_TOP = 12.0
TODAY_S = 10.0
def graph(title: str, top: float, colour: str) -> ui.echart:
    """Add one live graph: the time along the bottom, the values from 0 to `top` up the side."""
    return ui.echart({
        'animation': False,
        'color': [colour],
        'title': {'text': title, 'left': 0, 'textStyle': {'fontSize': 13}},
        'grid': {'left': 45, 'right': 15, 'top': 40, 'bottom': 28},
        'tooltip': {'trigger': 'axis'},
        'xAxis': {'type': 'time', 'axisLabel': {'hideOverlap': True}},
        'yAxis': {'type': 'value', 'min': 0, 'max': top},
        'series': [{'type': 'line', 'name': title, 'data': [], 'showSymbol': False}],
    }).classes(GRAPH_STYLE)


def day_graph() -> ui.echart:
    """Add the graph of the whole day: the clear-sky model, the estimated power and the estimated energy."""
    return ui.echart({
        'animation': False,
        'color': [MODEL_COLOUR, WATTS_COLOUR, ENERGY_COLOUR],
        'legend': {'right': 60, 'top': 0},
        'grid': {'left': 45, 'right': 55, 'top': 40, 'bottom': 28},
        'tooltip': {'trigger': 'axis'},
        'xAxis': {'type': 'time', 'axisLabel': {'hideOverlap': True}},
        'yAxis': [{'type': 'value', 'min': 0, 'max': DAY_WATTS_TOP, 'name': 'W'},
                  {'type': 'value', 'min': 0, 'name': 'Wh', 'splitLine': {'show': False}}],
        'series': [
            {'type': 'line', 'name': 'Clear-sky model in W', 'data': [], 'showSymbol': False,
             'lineStyle': {'type': 'dashed'}},
            {'type': 'line', 'name': 'Estimated power in W', 'data': [], 'showSymbol': False,
             'areaStyle': {'opacity': 0.2},
             'markPoint': {'symbol': 'circle', 'symbolSize': 12, 'data': [],
                           'label': {'show': True, 'formatter': 'measured', 'position': 'top'}}},
            {'type': 'line', 'name': 'Estimated energy in Wh', 'data': [], 'showSymbol': False, 'yAxisIndex': 1},
        ],
    }).classes(DAY_GRAPH_STYLE)
@ui.page('/')
def index() -> None:
    ui.label('Solar tracker dashboard').classes('text-2xl font-bold')
    bridge_status = ui.label()

    ui.label('Status').classes('font-bold')
    with ui.column().classes('gap-1'):
        with ui.row():
            state_line = ui.label()
            fault_line = ui.label()
        fault_help = ui.label().classes('whitespace-pre-line')
        problem_line = ui.label()
        moved_warning = ui.label().classes(FAULT_STYLE)

    ui.label('Panel').classes('font-bold')
    with ui.column().classes('gap-1'):
        with ui.row().classes('gap-8'):
            volts_label = ui.label()
            amps_label = ui.label()
            watts_label = ui.label()
        ui.label(power.NOISE_NOTE).classes(NOTE_STYLE)
    with ui.row().classes('w-full no-wrap items-start'):
        with ui.column().classes('w-80 shrink-0'):
            ui.label('Compass').classes('font-bold')
            compass_view = ui.html(compass.draw(None, None, None, None, None, False), sanitize=False)


        with ui.column().classes('grow min-w-0'):
            ui.label('Values (grey = old or never received)').classes('font-bold')
            value_labels = {}
            with ui.row():
                for key in KEYS:
                    value_labels[key] = ui.label()
            with ui.row().classes('gap-6'):
                with ui.column().classes('gap-1'):
                    ui.label('Side view').classes('font-bold')
                    side_view = ui.html(sideview.draw(None, None, False), sanitize=False)
                with ui.column().classes('gap-1'):
                    ui.label('Top view').classes('font-bold')
                    top_view = ui.html(topview.draw(None, None, None, None, False), sanitize=False)
    ui.label('Graphs (last 10 minutes)').classes('font-bold')
    with ui.row().classes('w-full no-wrap'):
        volts_graph = graph('Panel voltage U in V', VOLTS_TOP, VOLTS_COLOUR)
        amps_graph = graph('Panel current I in A', AMPS_TOP, AMPS_COLOUR)
        watts_graph = graph('Panel power P in W', WATTS_TOP, WATTS_COLOUR)

    ui.label('Today (estimate from the last 10 minutes)').classes('font-bold')
    with ui.row().classes('w-full no-wrap items-center gap-8'):
        with ui.column().classes('gap-1 shrink-0 w-72'):
            today_energy = ui.label().classes(PANEL_STYLE)
            today_kwh = ui.label().classes(NOTE_STYLE)
            today_note = ui.label().classes(NOTE_STYLE)
        today_graph = day_graph()
    ui.label('Messages').classes('font-bold')
    log = ui.log(max_lines=200).classes('w-full h-96 font-mono text-xs')
    seen = 0

    def refresh() -> None:
        nonlocal seen
        if link.connected:
            bridge_status.text = 'Bridge: connected'
        else:
            bridge_status.text = 'Bridge: not connected (start bridge.py)'

        now = time.monotonic()
        live = telemetry.value('ST', now) is not None

        def read(key: str):
            if live:
                return telemetry.value(key, now)
            return telemetry.last_value(key)

        fault = read('FLT')
        state_line.text = status.state_text(read('MODE'), read('IMU'), read('ST'), fault)
        fault_line.text = status.fault_text(fault)
        fault_help.text = status.fault_action(fault)
        text_style = '' if live else OLD_TEXT_STYLE
        fault_style = text_style
        help_style = text_style
        if live and status.has_fault(fault):
            fault_style = FAULT_STYLE
            help_style = FAULT_HELP_STYLE
        state_line.classes(replace=text_style)
        fault_line.classes(replace=fault_style)
        fault_help.classes(replace=f'whitespace-pre-line {help_style}')


        problem_line.text = status.problem_text(read('ERR'))
        problem_line.classes(replace=FAULT_STYLE if live else OLD_TEXT_STYLE)
        
        power_on = read('PAN')
        heading = read('HDG')
        panel = angles.panel_direction(read('IMU'), power_on, read('AZ'))
        compass_view.set_content(compass.draw(panel, read('SUN'), read('SEL'), power_on, heading, live))
        side_view.set_content(sideview.draw(read('EL'), read('SEL'), live))
        top_view.set_content(topview.draw(panel, power_on, read('SUN'), read('SEL'), live))
        moved_warning.text = ''
        if live and heading is not None and power_on is not None:
            moved_by = angles.difference(heading, power_on)
            if abs(moved_by) > angles.MOVED_DEG:
                moved_warning.text = status.moved_text(moved_by)

        panel_volts = power.volts(read('A0'))
        panel_amps = power.amps(read('A1'))
        panel_style = PANEL_STYLE if live else f'{PANEL_STYLE} {OLD_TEXT_STYLE}'
        volts_label.text = power.value_text('U', panel_volts, 1, 'V')
        amps_label.text = power.value_text('I', panel_amps, 2, 'A')
        watts_label.text = power.value_text('P', power.watts(panel_volts, panel_amps), 1, 'W')
        volts_label.classes(replace=panel_style)
        amps_label.classes(replace=panel_style)
        watts_label.classes(replace=panel_style)
        for key in KEYS:
            label = value_labels[key]
            current = telemetry.value(key, now)
            if current is not None:
                label.text = f'{key}={current}'
                label.classes(replace=CURRENT_STYLE)
            else:
                last = telemetry.last_value(key)
                label.text = f'{key}=?' if last is None else f'{key}={last}'
                label.classes(replace=OLD_STYLE)

        new_lines, seen = link.lines_since(seen)
        for line in new_lines:
            log.push(line)

    ui.timer(0.5, refresh)

    def draw_graphs() -> None:
        now = time.time()
        start, end = history.window(now)
        for chart, points in zip((volts_graph, amps_graph, watts_graph), history.series(now)):
            chart.options['xAxis']['min'] = start
            chart.options['xAxis']['max'] = end
            chart.options['series'][0]['data'] = points
            chart.update()

    ui.timer(1.0, draw_graphs)

    def draw_today() -> None:
        now = time.time()
        _, amps_points, watts_points = history.series(now)
        day_number, minutes = estimate.day_and_minutes(now)
        day_start = now - minutes * 60.0
        reason = estimate.why_not(amps_points, watts_points, sun.elevation(day_number, minutes))
        scale = None if reason else estimate.factor(watts_points)
        measured = estimate.mean(watts_points)
        energy_wh = estimate.day_wh(day_number, scale)
        model_points, power_points, energy_points = estimate.curves(day_number, day_start, scale)
        start, end = estimate.window(day_number, day_start)

        today_energy.text = estimate.energy_text(energy_wh)
        today_kwh.text = estimate.kwh_text(energy_wh)
        today_note.text = estimate.note_text(reason, scale, measured, clearsky.day_wh(day_number))
        today_graph.options['xAxis']['min'] = start
        today_graph.options['xAxis']['max'] = end
        today_graph.options['series'][0]['data'] = model_points
        today_graph.options['series'][1]['data'] = power_points
        today_graph.options['series'][2]['data'] = energy_points
        mark = [] if scale is None else [{'coord': [now * 1000.0, round(measured, 2)]}]
        today_graph.options['series'][1]['markPoint']['data'] = mark
        today_graph.update()

    ui.timer(TODAY_S, draw_today)
app.on_startup(link.start)

if __name__ in {'__main__', '__mp_main__'}:
    ui.run(host='127.0.0.1', port=8080, title='Solar tracker', show=False)