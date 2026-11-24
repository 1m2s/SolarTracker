"""Panel voltage, current and power for the dashboard.

A0 and A1 in the telemetry line are sums of 16 readings of the right Nano's
pins A0 and A1 (each reading 0 to 1023). This file turns the sums into volts,
amps and watts, following the circuit in Schaltplan_V1.pdf: A0 sits behind a
voltage divider (10 kOhm over 2.2 kOhm) across the panel, A1 sits above the
three 1 Ohm measuring resistors in parallel (1/3 Ohm).
"""

READINGS_PER_SUM = 16
READING_MAX = 1023
SUM_MAX = READINGS_PER_SUM * READING_MAX
REFERENCE_V = 5.0
DIVIDER_TOP_OHM = 10000.0
DIVIDER_BOTTOM_OHM = 2200.0
MEASURING_OHM = 1.0 / 3.0
UNKNOWN = '?'

NOISE_NOTE = ('Calculated from A0 and A1. With nothing plugged into the USB socket no current flows: '
              'a current up to about 0.05 A, and the power that goes with it, is measurement noise.')


def pin_volts(total: int | None) -> float | None:
    """Give back the average voltage at the Nano's pin for a sum of 16 readings.

    None if there is no sum or the sum is outside 0 to 16368.
    """
    if total is None or total < 0 or total > SUM_MAX:
        return None
    return total / READINGS_PER_SUM * REFERENCE_V / READING_MAX


def volts(sum_a0: int | None) -> float | None:
    """Give back the panel voltage in volts for the sum A0, or None if it cannot be known."""
    at_pin = pin_volts(sum_a0)
    if at_pin is None:
        return None
    return at_pin * (DIVIDER_TOP_OHM + DIVIDER_BOTTOM_OHM) / DIVIDER_BOTTOM_OHM


def amps(sum_a1: int | None) -> float | None:
    """Give back the panel current in amps for the sum A1, or None if it cannot be known."""
    at_pin = pin_volts(sum_a1)
    if at_pin is None:
        return None
    return at_pin / MEASURING_OHM


def watts(panel_volts: float | None, panel_amps: float | None) -> float | None:
    """Give back the power in watts, or None if the voltage or the current is not known."""
    if panel_volts is None or panel_amps is None:
        return None
    return panel_volts * panel_amps


def value_text(symbol: str, value: float | None, decimals: int, unit: str) -> str:
    """Give back a value with its unit, for example `U = 9.7 V`. None is shown as a question mark."""
    if value is None:
        return f'{symbol} = {UNKNOWN} {unit}'
    return f'{symbol} = {value:.{decimals}f} {unit}'