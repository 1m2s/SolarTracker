"""Tests for power.py. Run with `pytest` inside Dashboard/."""

from pytest import approx

import power


def test_volts_of_the_daylight_readings():
    assert power.volts(5705) == approx(9.66, abs=0.01)
    assert power.volts(7880) == approx(13.35, abs=0.01)


def test_volts_of_the_room_light_reading():
    assert power.volts(2629) == approx(4.45, abs=0.01)


def test_volts_of_the_sticker_values():
    assert power.volts(6672) == approx(11.3, abs=0.01)
    assert power.volts(5424) == approx(9.19, abs=0.01)


def test_volts_at_both_ends_of_the_range():
    assert power.volts(0) == 0.0
    assert power.volts(16368) == approx(5.0 * 12.2 / 2.2)


def test_amps_of_the_sticker_values():
    assert power.amps(1280) == approx(1.173, abs=0.001)
    assert power.amps(1184) == approx(1.085, abs=0.001)


def test_amps_of_the_noise_with_an_empty_socket():
    assert power.amps(19) == approx(0.017, abs=0.001)
    assert power.amps(48) == approx(0.044, abs=0.001)


def test_amps_at_both_ends_of_the_range():
    assert power.amps(0) == 0.0
    assert power.amps(16368) == approx(15.0)


def test_no_sum_gives_no_value():
    assert power.volts(None) is None
    assert power.amps(None) is None


def test_sum_outside_the_range_gives_no_value():
    assert power.volts(-1) is None
    assert power.volts(16369) is None
    assert power.amps(-1) is None
    assert power.amps(16369) is None


def test_watts_is_volts_times_amps():
    assert power.watts(9.2, 1.09) == approx(10.028)
    assert power.watts(13.3, 0.0) == 0.0


def test_watts_needs_both_values():
    assert power.watts(None, 1.09) is None
    assert power.watts(9.2, None) is None
    assert power.watts(None, None) is None


def test_value_text_shows_the_unit_and_the_decimals():
    assert power.value_text('U', 9.664, 1, 'V') == 'U = 9.7 V'
    assert power.value_text('I', 0.0046, 2, 'A') == 'I = 0.00 A'
    assert power.value_text('P', 10.028, 1, 'W') == 'P = 10.0 W'


def test_value_text_of_an_unknown_value():
    assert power.value_text('U', None, 1, 'V') == 'U = ? V'