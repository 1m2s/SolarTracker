"""Tests for sun.py. Run with `pytest` inside Dashboard/.

The expected numbers come from the firmware's own sun maths (AutoFirmware,
sunPosition) run on the PC for the same day and time.
"""

from pytest import approx

import sun


def test_day_of_year_at_both_ends():
    assert sun.day_of_year(1, 1) == 1
    assert sun.day_of_year(12, 31) == 365


def test_day_of_year_of_days_in_the_logs():
    assert sun.day_of_year(11, 24) == 328
    assert sun.day_of_year(6, 21) == 172
    assert sun.day_of_year(3, 1) == 60


def test_cet_minutes_in_winter_time():
    assert sun.cet_minutes(15, 57, False) == 957
    assert sun.cet_minutes(0, 0, False) == 0


def test_cet_minutes_takes_one_hour_off_in_summer_time():
    assert sun.cet_minutes(13, 0, True) == 720


def test_declination_matches_the_firmware():
    assert sun.declination(328) == approx(-20.4225, abs=0.001)
    assert sun.declination(172) == approx(23.4437, abs=0.001)
    assert sun.declination(79) == approx(-0.4135, abs=0.001)


def test_equation_of_time_matches_the_firmware():
    assert sun.equation_of_time(328) == approx(13.1756, abs=0.001)
    assert sun.equation_of_time(172) == approx(-1.6073, abs=0.001)
    assert sun.equation_of_time(79) == approx(-7.6501, abs=0.001)


def test_elevation_of_the_matlab_line_on_24_nov():
    assert sun.elevation(328, sun.cet_minutes(15, 57, False)) == approx(4.7245, abs=0.01)


def test_elevation_of_the_boot_test_on_12_nov():
    assert sun.elevation(316, sun.cet_minutes(13, 24, False)) == approx(22.3430, abs=0.01)


def test_elevation_is_negative_at_night():
    assert sun.elevation(314, sun.cet_minutes(19, 14, False)) == approx(-23.9461, abs=0.01)
    assert sun.elevation(1, 0) == approx(-64.7517, abs=0.01)


def test_elevation_at_noon_in_june_and_december():
    assert sun.elevation(172, 720) == approx(65.2079, abs=0.01)
    assert sun.elevation(355, 720) == approx(18.6432, abs=0.01)


def test_elevation_is_the_same_for_summer_and_winter_clock():
    assert sun.elevation(172, sun.cet_minutes(13, 0, True)) == approx(sun.elevation(172, 720))


def test_elevation_near_sunrise_in_march():
    assert sun.elevation(79, sun.cet_minutes(6, 30, False)) == approx(-0.1402, abs=0.01)