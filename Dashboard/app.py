from nicegui import ui
from serial.tools import list_ports


@ui.page('/')
def index() -> None:
    ui.label('Solar tracker dashboard').classes('text-2xl font-bold')
    ui.label('Serial ports found on this laptop:')
    ports = list_ports.comports()
    if not ports:
        ui.label('none - is the USB cable plugged in?')
    for port in ports:
        ui.label(f'{port.device}  ({port.description})').classes('font-mono')


ui.run(host='127.0.0.1', port=8080, title='Solar tracker', reload=False, show=False)