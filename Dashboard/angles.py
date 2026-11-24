"""Angle maths for the dashboard. All angles are in degrees from true north, clockwise."""

FULL_TURN = 360.0
HALF_TURN = 180.0
MOVED_DEG = 5.0


def wrap(deg: float) -> float:
    """Bring an angle into the range 0 (included) to 360 (not included)."""
    return deg % FULL_TURN


def difference(a: float, b: float) -> float:
    """Give back a - b the short way round, from -180 (included) to 180 (not included)."""
    return (a - b + HALF_TURN) % FULL_TURN - HALF_TURN


def panel_direction(imu: int | None, pan: float | None, az: float | None) -> float | None:
    """Give back the direction the panel faces now, or None if it cannot be known.

    imu: IMU switch (1 AN, 0 AUS). pan: direction at power-on. az: azimuth last
    sent to the right Nano, where 180 means the direction at power-on.
    """
    if imu is None or az is None:
        return None
    if imu == 0:
        return wrap(az)
    if pan is None:
        return None
    return wrap(pan + az - HALF_TURN)