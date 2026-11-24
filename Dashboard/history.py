"""The last minutes of panel voltage, current and power, for the graphs.

Every telemetry line adds one point. Points older than WINDOW_S are dropped.
A point without a reading, and a pause longer than GAP_S between two points,
break the line in the graph, so it is not drawn across a time without data.
The points are kept in memory only: they are gone when app.py stops.
"""

import threading
from collections import deque

import power

WINDOW_S = 600.0
GAP_S = 3.0
MS_PER_S = 1000.0
VOLT_DECIMALS = 2
AMP_DECIMALS = 3
WATT_DECIMALS = 2

points = deque()
lock = threading.Lock()


def drop_old(now: float) -> None:
    """Remove the points that are older than WINDOW_S. Call only while holding the lock."""
    while points and now - points[0][0] > WINDOW_S:
        points.popleft()


def add(now: float, sum_a0: int | None, sum_a1: int | None) -> None:
    """Add one point for the clock time `now` (seconds) from the sums A0 and A1.

    After a pause longer than GAP_S an empty point is added first, which
    breaks the line in the graph.
    """
    panel_volts = power.volts(sum_a0)
    panel_amps = power.amps(sum_a1)
    with lock:
        if points and now - points[-1][0] > GAP_S:
            points.append((now, None, None, None))
        points.append((now, panel_volts, panel_amps, power.watts(panel_volts, panel_amps)))
        drop_old(now)


def pair(seconds: float, value: float | None, decimals: int) -> list:
    """Give back one graph point: the time in milliseconds and the rounded value (or None)."""
    return [seconds * MS_PER_S, None if value is None else round(value, decimals)]


def series(now: float) -> tuple[list, list, list]:
    """Give back the points of the last WINDOW_S seconds as three lists: volts, amps, watts."""
    with lock:
        drop_old(now)
        kept = list(points)
    return ([pair(seconds, panel_volts, VOLT_DECIMALS) for seconds, panel_volts, _, _ in kept],
            [pair(seconds, panel_amps, AMP_DECIMALS) for seconds, _, panel_amps, _ in kept],
            [pair(seconds, panel_watts, WATT_DECIMALS) for seconds, _, _, panel_watts in kept])


def window(now: float) -> tuple[float, float]:
    """Give back the start and the end of the graphs' time axis, in milliseconds."""
    return (now - WINDOW_S) * MS_PER_S, now * MS_PER_S


def clear() -> None:
    """Forget all points."""
    with lock:
        points.clear()