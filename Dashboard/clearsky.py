"""Clear-sky model: the power the panel would give under a cloudless sky.

The panel follows the sun, so it always faces it. Then the light on the panel
depends only on how much air the sunlight has to cross, and that depends on
how high the sun stands:

1. air_mass: how many times more air the light crosses than with the sun
   straight overhead (formula of Kasten and Young, 1989).
2. direct_light: the direct sunlight that is left after that air, in W/m2
   (formula of Meinel and Meinel, 1976, as used on pveducation.org).
3. panel_watts: the panel's power in that light. Light scattered by the sky
   adds about 10 %, and the sticker's 10 W holds for 1000 W/m2.

This is a model, not a measurement: no clouds, no haze, and a panel that
gives power in proportion to the light.
"""

import math

import sun

SOLAR_CONSTANT_W_M2 = 1353.0
CLEAR_AIR_SHARE = 0.7
AIR_MASS_POWER = 0.678
SKY_LIGHT_FACTOR = 1.1
STICKER_LIGHT_W_M2 = 1000.0
PANEL_RATED_W = 10.0
MINUTES_PER_DAY = 1440
MINUTES_PER_HOUR = 60.0


def air_mass(elevation_deg: float) -> float | None:
    """Give back the air mass for a sun height in degrees, or None when the sun is not up."""
    if elevation_deg <= 0.0:
        return None
    return 1.0 / (math.sin(math.radians(elevation_deg)) + 0.50572 * (elevation_deg + 6.07995) ** -1.6364)


def direct_light(elevation_deg: float) -> float:
    """Give back the direct sunlight on a surface facing the sun, in W/m2 (0 when the sun is not up)."""
    mass = air_mass(elevation_deg)
    if mass is None:
        return 0.0
    return SOLAR_CONSTANT_W_M2 * CLEAR_AIR_SHARE ** (mass ** AIR_MASS_POWER)


def panel_watts(elevation_deg: float) -> float:
    """Give back the panel's clear-sky power in watts for a sun height in degrees."""
    return PANEL_RATED_W * SKY_LIGHT_FACTOR * direct_light(elevation_deg) / STICKER_LIGHT_W_M2


def watts_at(day_number: int, minutes: float) -> float:
    """Give back the panel's clear-sky power in watts for a day of the year and a time.

    `minutes` are minutes since midnight in winter time (CET).
    """
    return panel_watts(sun.elevation(day_number, minutes))


def day_wh(day_number: int) -> float:
    """Give back the clear-sky energy of one whole day in Wh, added up minute by minute."""
    return sum(watts_at(day_number, minute) for minute in range(MINUTES_PER_DAY)) / MINUTES_PER_HOUR