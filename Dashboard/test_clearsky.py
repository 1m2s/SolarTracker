"""Tests for clearsky.py. Run with `pytest` inside Dashboard/.

The expected numbers were calculated by hand from the two formulas
(Kasten and Young for the air mass, Meinel and Meinel for the light).
"""

from pytest import approx

import clearsky


def test_air_mass_is_one_with_the_sun_overhead():
    assert clearsky.air_mass(90.0) == approx(1.0, abs=0.001)


def test_air_mass_is_about_two_at_30_degrees():
    assert clearsky.air_mass(30.0) == approx(1.994, abs=0.001)


def test_air_mass_grows_when_the_sun_is_lower():
    assert clearsky.air_mass(10.0) == approx(5.586, abs=0.001)
    assert clearsky.air_mass(5.0) == approx(10.306, abs=0.001)


def test_air_mass_is_unknown_when_the_sun_is_not_up():
    assert clearsky.air_mass(0.0) is None
    assert clearsky.air_mass(-12.0) is None


def test_direct_light_with_the_sun_overhead_is_70_percent_of_the_solar_constant():
    assert clearsky.direct_light(90.0) == approx(1353.0 * 0.7, abs=0.5)


def test_direct_light_at_lower_sun_heights():
    assert clearsky.direct_light(30.0) == approx(765.5, abs=0.5)
    assert clearsky.direct_light(10.0) == approx(430.6, abs=0.5)


def test_direct_light_is_zero_when_the_sun_is_not_up():
    assert clearsky.direct_light(0.0) == 0.0
    assert clearsky.direct_light(-5.0) == 0.0


def test_panel_watts_adds_the_sky_light_and_uses_the_sticker_power():
    assert clearsky.panel_watts(90.0) == approx(10.0 * 1.1 * 947.2 / 1000.0, abs=0.01)
    assert clearsky.panel_watts(30.0) == approx(8.42, abs=0.01)


def test_panel_watts_is_zero_at_night():
    assert clearsky.panel_watts(-20.0) == 0.0


def test_watts_at_noon_and_in_the_afternoon_on_24_nov():
    assert clearsky.watts_at(328, 720) == approx(7.42, abs=0.01)
    assert clearsky.watts_at(328, 957) == approx(2.49, abs=0.01)


def test_watts_at_night_is_zero():
    assert clearsky.watts_at(328, 180) == 0.0


def test_day_wh_on_24_nov():
    assert clearsky.day_wh(328) == approx(48.0, abs=0.1)


def test_day_wh_of_the_longest_and_the_shortest_day():
    assert clearsky.day_wh(172) == approx(125.0, abs=0.1)
    assert clearsky.day_wh(355) == approx(41.3, abs=0.1)


def test_day_wh_is_higher_in_summer_than_in_winter():
    assert clearsky.day_wh(172) > clearsky.day_wh(79) > clearsky.day_wh(328) > clearsky.day_wh(355)