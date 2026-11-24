"""How high the sun stands over Weingarten at a given day and time.

The same maths as the left Nano uses (AutoFirmware, SolarTime.cpp and
SunAngles.cpp), which came from the MATLAB functions JahresTag, Deklination,
Zeitgleichung, MOZ, StundenWinkel and HoehenWinkel. Only the height
(elevation) is calculated here, not the direction (azimuth).

Times are minutes since midnight in winter time (CET), like the Nano's clock.
"""

import math

LATITUDE_DEG = 47.81
LONGITUDE_DEG = 9.66
CET_MERIDIAN_DEG = 15.0
MINUTES_PER_DEGREE = 4.0
DAYS_PER_YEAR = 365.0
DAYS_IN_MONTH = (31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31)


def day_of_year(month: int, day: int) -> int:
    """Give back the number of the day in the year: 1 January is 1, 31 December is 365."""
    return sum(DAYS_IN_MONTH[:month - 1]) + day


def cet_minutes(hour: int, minute: int, summer_time: bool) -> int:
    """Give back the minutes since midnight in winter time for a clock time."""
    if summer_time:
        hour = hour - 1
    return hour * 60 + minute


def declination(day_number: int) -> float:
    """Give back how far north (+) or south (-) of the equator the sun stands, in degrees."""
    year_angle = 360.0 * day_number / DAYS_PER_YEAR
    return (0.3948
            - 23.2559 * math.cos(math.radians(year_angle + 9.1))
            - 0.391 * math.cos(math.radians(2.0 * year_angle + 5.4))
            - 0.1764 * math.cos(math.radians(3.0 * year_angle + 26.0)))


def equation_of_time(day_number: int) -> float:
    """Give back how many minutes the sun is ahead of (+) or behind (-) an even clock."""
    year_angle = 360.0 * day_number / DAYS_PER_YEAR
    return (0.0066
            + 7.3525 * math.cos(math.radians(year_angle + 85.9))
            + 9.9359 * math.cos(math.radians(2.0 * year_angle + 108.9))
            + 0.3387 * math.cos(math.radians(3.0 * year_angle + 105.2)))


def elevation(day_number: int, minutes: float) -> float:
    """Give back the sun's height above the horizon in degrees (negative = below).

    `minutes` are minutes since midnight in winter time (CET).
    """
    mean_local = minutes - MINUTES_PER_DEGREE * (CET_MERIDIAN_DEG - LONGITUDE_DEG)
    true_solar = mean_local + equation_of_time(day_number)
    hour_angle = math.radians((12.0 - true_solar / 60.0) * 15.0)
    latitude = math.radians(LATITUDE_DEG)
    sun_declination = math.radians(declination(day_number))
    return math.degrees(math.asin(math.cos(hour_angle) * math.cos(latitude) * math.cos(sun_declination)
                                  + math.sin(latitude) * math.sin(sun_declination)))