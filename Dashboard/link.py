"""Connection from the dashboard to bridge.py.

A background thread reads the Nano's lines from the bridge and keeps the
latest ones. Telemetry lines are handed to telemetry.py and kept apart from
the normal messages. The UI only reads from here; it never touches the
serial port.
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
MAX_TELEMETRY_LINES = 5

connected = False
lines = deque(maxlen=MAX_LINES)
line_count = 0
telemetry_lines = deque(maxlen=MAX_TELEMETRY_LINES)
lock = threading.Lock()


def store(line: str) -> None:
    """Keep one line from the Nano: telemetry in its own short list, the rest as messages."""
    global line_count
    is_telemetry = telemetry.update(line, time.monotonic())
    with lock:
        if is_telemetry:
            telemetry_lines.append(line)
        else:
            lines.append(line)
            line_count += 1


def lines_since(seen: int) -> tuple[list[str], int]:
    """Give back the messages that arrived after the first `seen` ones, and the new count."""
    with lock:
        missed = min(line_count - seen, len(lines))
        new_lines = list(lines)[len(lines) - missed:]
        return new_lines, line_count


def latest_telemetry_lines() -> list[str]:
    """Give back the last few telemetry lines, oldest first."""
    with lock:
        return list(telemetry_lines)


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