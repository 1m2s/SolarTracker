"""Estimate of the energy of the whole day, from the last minutes of measured power.

The page cannot run for a whole day, so it extrapolates:

1. clearsky.py says what the panel would give under a cloudless sky at any
   minute of today.
2. The power measured over the last minutes (history.py) is divided by the
   model's power at the same times. That gives one factor, for example 0.78.
3. The model's curve for the whole day is multiplied by that factor and
   added up to an energy in Wh.

This is an estimate, not a measurement. It holds only if the sky and the load
stay as they are now. There is no estimate without a load (the current is
then only noise) and none when the sun is low (the model's power is then so
small that the factor jumps).
"""

import time

import clearsky
import sun

MIN_SUN_DEG = 10.0
MIN_LOAD_A = 0.05
STEP_MIN = 5
MARGIN_MIN = 30
MINUTES_PER_DAY = 1440
SECONDS_PER_MINUTE = 60.0
MINUTES_PER_HOUR = 60.0
MS_PER_S = 1000.0
WH_PER_KWH = 1000.0

NO_READING = 'there is no power reading'
NO_LOAD = f'nothing is plugged into the USB socket (current below {MIN_LOAD_A} A)'
SUN_TOO_LOW = f'the sun is lower than {MIN_SUN_DEG:.0f}°'


def day_and_minutes(clock_seconds: float) -> tuple[int, float]:
    """Give back the day of the year and the minutes since midnight in winter time for a clock time."""
    clock = time.localtime(clock_seconds)
    minutes = sun.cet_minutes(clock.tm_hour, clock.tm_min, clock.tm_isdst > 0) + clock.tm_sec / SECONDS_PER_MINUTE
    return sun.day_of_year(clock.tm_mon, clock.tm_mday), minutes


def mean(points: list) -> float | None:
    """Give back the average value of graph points `[milliseconds, value]`, leaving out the empty ones."""
    values = [value for _, value in points if value is not None]
    if not values:
        return None
    return sum(values) / len(values)


def why_not(amps_points: list, watts_points: list, sun_deg: float) -> str:
    """Give back why there is no estimate right now, or an empty text if there is one."""
    if mean(watts_points) is None:
        return NO_READING
    load = mean(amps_points)
    if load is None or load < MIN_LOAD_A:
        return NO_LOAD
    if sun_deg < MIN_SUN_DEG:
        return SUN_TOO_LOW
    return ''


def factor(watts_points: list) -> float | None:
    """Give back the measured power divided by the model's power over the same points, or None."""
    measured = 0.0
    model = 0.0
    for milliseconds, value in watts_points:
        if value is None:
            continue
        day_number, minutes = day_and_minutes(milliseconds / MS_PER_S)
        measured += value
        model += clearsky.watts_at(day_number, minutes)
    if model <= 0.0:
        return None
    return measured / model


def day_wh(day_number: int, scale: float | None) -> float | None:
    """Give back the estimated energy of the whole day in Wh, or None without a factor."""
    if scale is None:
        return None
    return scale * clearsky.day_wh(day_number)


def curves(day_number: int, day_start_s: float, scale: float | None) -> tuple[list, list, list]:
    """Give back three lists of graph points for the whole day, one point every STEP_MIN minutes.

    The model's power in W, the estimated power in W and the estimated energy
    so far in Wh. `day_start_s` is the clock time of minute 0. Without a factor
    the last two lists are empty.
    """
    model_points = []
    power_points = []
    energy_points = []
    energy = 0.0
    for minute in range(MINUTES_PER_DAY):
        model = clearsky.watts_at(day_number, minute)
        if scale is not None:
            energy += scale * model / MINUTES_PER_HOUR
        if minute % STEP_MIN != 0:
            continue
        milliseconds = (day_start_s + minute * SECONDS_PER_MINUTE) * MS_PER_S
        model_points.append([milliseconds, round(model, 2)])
        if scale is not None:
            power_points.append([milliseconds, round(scale * model, 2)])
            energy_points.append([milliseconds, round(energy, 2)])
    return model_points, power_points, energy_points


def window(day_number: int, day_start_s: float) -> tuple[float, float]:
    """Give back the start and the end of the day graph's time axis, in milliseconds.

    From MARGIN_MIN minutes before the sun comes up to MARGIN_MIN minutes after it goes down.
    """
    sun_up = [minute for minute in range(MINUTES_PER_DAY) if sun.elevation(day_number, minute) > 0.0]
    first = sun_up[0] - MARGIN_MIN
    last = sun_up[-1] + MARGIN_MIN
    return ((day_start_s + first * SECONDS_PER_MINUTE) * MS_PER_S,
            (day_start_s + last * SECONDS_PER_MINUTE) * MS_PER_S)


def energy_text(energy_wh: float | None) -> str:
    """Give back the headline, for example `E ≈ 36 Wh`. None is shown as a question mark."""
    if energy_wh is None:
        return 'E ≈ ? Wh'
    return f'E ≈ {energy_wh:.0f} Wh'


def kwh_text(energy_wh: float | None) -> str:
    """Give back the same energy in kWh as a short line, or an empty text without an estimate."""
    if energy_wh is None:
        return ''
    return f'= {energy_wh / WH_PER_KWH:.3f} kWh for the whole day'


def note_text(reason: str, scale: float | None, measured_w: float | None, clear_wh: float) -> str:
    """Give back the note under the headline: how the estimate was made, or why there is none."""
    if reason or scale is None or measured_w is None:
        return (f'No estimate: {reason or NO_READING}. '
                f'The dashed line is the clear-sky model for today ({clear_wh:.0f} Wh).')
    return (f'Estimate, not measured. Over the last 10 minutes the panel gave {measured_w:.1f} W, '
            f'{scale * 100.0:.0f} % of the clear-sky model. The model for today ({clear_wh:.0f} Wh) is '
            'scaled by that. It holds only if the sky and the load stay as they are now.')