from nicegui import app, ui

import link


@ui.page('/')
def index() -> None:
    ui.label('Solar tracker dashboard').classes('text-2xl font-bold')
    status = ui.label()
    log = ui.log(max_lines=200).classes('w-full h-96 font-mono text-xs')
    seen = 0

    def refresh() -> None:
        nonlocal seen
        if link.connected:
            status.text = 'Bridge: connected'
        else:
            status.text = 'Bridge: not connected (start bridge.py)'
        new_lines, seen = link.lines_since(seen)
        for line in new_lines:
            log.push(line)

    ui.timer(0.5, refresh)


app.on_startup(link.start)

if __name__ in {'__main__', '__mp_main__'}:
    ui.run(host='127.0.0.1', port=8080, title='Solar tracker', show=False)