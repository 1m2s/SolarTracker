"""Connection from the dashboard to bridge.py.

A background thread reads the Nano's lines from the bridge. Telemetry lines
are handed to telemetry.py; the normal messages are kept here, latest ones
only. The UI only reads from here; it never touches the serial port.
"""

import socket
import threading
import time
from collections import deque

import telemetry

HOST = '127.0.0.1'
TCP_PORT = 5555
RETRY_S = 2.0
MAX_LINES = 500

connected = False
lines = deque(maxlen=MAX_LINES)
line_count = 0
lock = threading.Lock()


def store(line: str) -> None:
    """Take one line from the Nano: telemetry goes to telemetry.py, the rest is kept as a message."""
    global line_count
    if telemetry.update(line, time.monotonic()):
        return
    with lock:
        lines.append(line)
        line_count += 1


def lines_since(seen: int) -> tuple[list[str], int]:
    """Give back the messages that arrived after the first `seen` ones, and the new count."""
    with lock:
        missed = min(line_count - seen, len(lines))
        new_lines = list(lines)[len(lines) - missed:]
        return new_lines, line_count


def run() -> None:
    """Connect to the bridge, read lines forever, reconnect when the bridge is gone."""
    global connected
    while True:
        try:
            with socket.create_connection((HOST, TCP_PORT), timeout=RETRY_S) as bridge:
                bridge.settimeout(None)
                connected = True
                buffer = b''
                while True:
                    data = bridge.recv(1024)
                    if not data:
                        break
                    buffer += data
                    while b'\n' in buffer:
                        line, buffer = buffer.split(b'\n', 1)
                        store(line.decode(errors='replace').rstrip())
        except OSError:
            pass
        connected = False
        time.sleep(RETRY_S)


def start() -> None:
    """Start the background thread."""
    threading.Thread(target=run, daemon=True).start()