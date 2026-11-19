import time

from nicegui import app, ui

import angles
import compass
import link
import status
import telemetry

KEYS = ('MODE', 'ST', 'FLT', 'ERR', 'IMU', 'MOV', 'SUN', 'SEL', 'PAN', 'HDG', 'AZ', 'EL', 'A0', 'A1')
CURRENT_STYLE = 'font-mono text-sm'
OLD_STYLE = 'font-mono text-sm text-gray-400'
FAULT_STYLE = 'font-bold text-red-700'
FAULT_HELP_STYLE = 'text-red-700'
OLD_TEXT_STYLE = 'text-gray-400'


@ui.page('/')
def index() -> None:
    ui.label('Solar tracker dashboard').classes('text-2xl font-bold')
    bridge_status = ui.label()

    ui.label('Status').classes('font-bold')
    with ui.row():
        state_line = ui.label()
        fault_line = ui.label()
    fault_help = ui.label().classes('whitespace-pre-line')

    with ui.row().classes('w-full no-wrap items-start'):
        with ui.column().classes('w-80 shrink-0'):
            ui.label('Compass').classes('font-bold')
            compass_view = ui.html(compass.draw(None, None, None, None, None, False), sanitize=False)
            moved_warning = ui.label().classes(FAULT_STYLE)

        with ui.column().classes('grow min-w-0'):
            ui.label('Values (grey = old or never received)').classes('font-bold')
            value_labels = {}
            with ui.row():
                for key in KEYS:
                    value_labels[key] = ui.label()

            ui.label('Telemetry lines').classes('font-bold')
            telemetry_box = ui.label().classes('w-full h-24 p-2 border rounded font-mono text-xs whitespace-pre overflow-auto')

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

        power_on = read('PAN')
        heading = read('HDG')
        panel = angles.panel_direction(read('IMU'), power_on, read('AZ'))
        compass_view.set_content(compass.draw(panel, read('SUN'), read('SEL'), power_on, heading, live))

        moved_warning.text = ''
        if live and heading is not None and power_on is not None:
            moved_by = angles.difference(heading, power_on)
            if abs(moved_by) > angles.MOVED_DEG:
                moved_warning.text = status.moved_text(moved_by)

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

        telemetry_box.text = '\n'.join(link.latest_telemetry_lines())

        new_lines, seen = link.lines_since(seen)
        for line in new_lines:
            log.push(line)

    ui.timer(0.5, refresh)


app.on_startup(link.start)

if __name__ in {'__main__', '__mp_main__'}:
    ui.run(host='127.0.0.1', port=8080, title='Solar tracker', show=False)