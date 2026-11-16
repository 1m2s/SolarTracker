import time

from nicegui import app, ui

import link
import telemetry

KEYS = ('MODE', 'ST', 'FLT', 'IMU', 'MOV', 'SUN', 'SEL', 'PAN', 'HDG', 'AZ', 'EL')
CURRENT_STYLE = 'font-mono text-sm'
OLD_STYLE = 'font-mono text-sm text-gray-400'


@ui.page('/')
def index() -> None:
    ui.label('Solar tracker dashboard').classes('text-2xl font-bold')
    status = ui.label()

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
            status.text = 'Bridge: connected'
        else:
            status.text = 'Bridge: not connected (start bridge.py)'

        now = time.monotonic()
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