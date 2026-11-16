"""Telemetry from the left Nano.

Turns a line that starts with `T:` into values and keeps the latest value
of every key together with the time it arrived, so the page can tell a
current value from an old one.
"""

import math
import threading

PREFIX = 'T:'
MODES = ('A', 'M')
WHOLE_KEYS = ('ST', 'FLT', 'IMU', 'MOV')
DEGREE_KEYS = ('SUN', 'SEL', 'PAN', 'HDG', 'AZ', 'EL')
MAX_AGE_S = 3.0

latest = {}
lock = threading.Lock()


def parse(line: str) -> dict[str, str | int | float] | None:
    """Give back the values in one telemetry line, or None if the line is not telemetry."""
    if not line.startswith(PREFIX):
        return None
    values = {}
    for pair in line[len(PREFIX):].split():
        key, equals, text = pair.partition('=')
        if not equals:
            continue
        try:
            if key == 'MODE':
                if text in MODES:
                    values[key] = text
            elif key in WHOLE_KEYS:
                values[key] = int(text)
            elif key in DEGREE_KEYS:
                number = float(text)
                if math.isfinite(number):
                    values[key] = number
        except ValueError:
            continue
    return values


def update(line: str, now: float) -> bool:
    """Store the values of one line with the time `now`. True if the line was telemetry."""
    values = parse(line)
    if values is None:
        return False
    with lock:
        for key, value in values.items():
            latest[key] = (value, now)
    return True


def value(key: str, now: float) -> str | int | float | None:
    """Give back the value of `key` if it arrived in the last MAX_AGE_S seconds, else None."""
    with lock:
        entry = latest.get(key)
    if entry is None:
        return None
    stored, seen_at = entry
    if now - seen_at > MAX_AGE_S:
        return None
    return stored


def last_value(key: str) -> str | int | float | None:
    """Give back the last value of `key`, however old, or None if it never arrived."""
    with lock:
        entry = latest.get(key)
    if entry is None:
        return None
    return entry[0]


def clear() -> None:
    """Forget all values."""
    with lock:
        latest.clear()