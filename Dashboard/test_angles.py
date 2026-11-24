"""Tests for angles.py. Run with `pytest` inside Dashboard/."""

from pytest import approx

import angles


def test_wrap_keeps_angles_that_are_already_in_range():
    assert angles.wrap(0.0) == 0.0
    assert angles.wrap(304.6) == approx(304.6)


def test_wrap_brings_large_and_negative_angles_into_range():
    assert angles.wrap(360.0) == 0.0
    assert angles.wrap(370.0) == approx(10.0)
    assert angles.wrap(725.0) == approx(5.0)
    assert angles.wrap(-10.0) == approx(350.0)


def test_difference_of_close_angles():
    assert angles.difference(304.4, 304.6) == approx(-0.2)
    assert angles.difference(304.6, 304.4) == approx(0.2)


def test_difference_goes_the_short_way_across_north():
    assert angles.difference(10.0, 350.0) == approx(20.0)
    assert angles.difference(350.0, 10.0) == approx(-20.0)


def test_difference_of_opposite_directions_is_minus_180():
    assert angles.difference(0.0, 180.0) == approx(-180.0)
    assert angles.difference(180.0, 0.0) == approx(-180.0)


def test_panel_direction_with_imu_an():
    assert angles.panel_direction(1, 304.6, 180.0) == approx(304.6)
    assert angles.panel_direction(1, 304.6, 56.0) == approx(180.6)


def test_panel_direction_with_imu_an_wraps_past_north():
    assert angles.panel_direction(1, 350.0, 200.0) == approx(10.0)
    assert angles.panel_direction(1, 10.0, 150.0) == approx(340.0)


def test_panel_direction_with_imu_aus_is_the_azimuth():
    assert angles.panel_direction(0, None, 180.0) == approx(180.0)
    assert angles.panel_direction(0, 304.6, 200.0) == approx(200.0)


def test_panel_direction_is_unknown_without_its_inputs():
    assert angles.panel_direction(None, 304.6, 56.0) is None
    assert angles.panel_direction(1, None, 56.0) is None
    assert angles.panel_direction(1, 304.6, None) is None
    assert angles.panel_direction(0, None, None) is None