"""Tests for estimate.py. Run with `pytest` inside Dashboard/."""

import time

from pytest import approx

import clearsky
import estimate

NOV_24 = 328
MIDNIGHT_24_NOV = time.mktime((2026, 11, 24, 0, 0, 0, 0, 0, -1))


def points_at_noon(share: float) -> list:
    """Ten minutes of graph points around 11:25 on 24 Nov, each `share` of the model's power."""
    points = []
    for second in range(0, 600, 5):
        minutes = 680.0 + second / 60.0
        watts = share * clearsky.watts_at(NOV_24, minutes)
        points.append([(MIDNIGHT_24_NOV + minutes * 60.0) * 1000.0, watts])
    return points


def test_day_and_minutes_of_a_clock_time():
    clock = time.mktime((2026, 11, 24, 15, 57, 30, 0, 0, -1))
    assert estimate.day_and_minutes(clock) == (328, approx(957.5))


def test_mean_leaves_out_empty_points():
    assert estimate.mean([[0, 2.0], [1, None], [2, 4.0]]) == approx(3.0)


def test_mean_of_nothing_is_none():
    assert estimate.mean([]) is None
    assert estimate.mean([[0, None]]) is None


def test_why_not_is_empty_with_a_load_and_a_high_sun():
    assert estimate.why_not([[0, 0.9]], [[0, 8.0]], 20.0) == ''


def test_why_not_without_a_reading():
    assert estimate.why_not([], [], 20.0) == estimate.NO_READING
    assert estimate.why_not([[0, None]], [[0, None]], 20.0) == estimate.NO_READING


def test_why_not_without_a_load():
    assert estimate.why_not([[0, 0.02]], [[0, 0.3]], 20.0) == estimate.NO_LOAD


def test_why_not_with_a_low_sun():
    assert estimate.why_not([[0, 0.9]], [[0, 2.0]], 9.9) == estimate.SUN_TOO_LOW
    assert estimate.why_not([[0, 0.9]], [[0, 2.0]], 10.0) == ''


def test_why_not_names_the_missing_load_before_the_low_sun():
    assert estimate.why_not([[0, 0.02]], [[0, 0.3]], 3.0) == estimate.NO_LOAD


def test_factor_is_the_share_of_the_model():
    assert estimate.factor(points_at_noon(0.78)) == approx(0.78, abs=0.001)
    assert estimate.factor(points_at_noon(1.0)) == approx(1.0, abs=0.001)


def test_factor_leaves_out_empty_points():
    points = points_at_noon(0.5)
    points[3][1] = None
    assert estimate.factor(points) == approx(0.5, abs=0.001)


def test_factor_is_none_without_points_and_at_night():
    assert estimate.factor([]) is None
    assert estimate.factor([[(MIDNIGHT_24_NOV + 3 * 3600) * 1000.0, 0.2]]) is None


def test_day_wh_scales_the_model():
    assert estimate.day_wh(NOV_24, 0.78) == approx(0.78 * 48.0, abs=0.1)
    assert estimate.day_wh(NOV_24, None) is None


def test_curves_have_one_point_every_five_minutes():
    model, power, energy = estimate.curves(NOV_24, 1000.0, 0.78)
    assert len(model) == len(power) == len(energy) == 288
    assert model[0] == [1000000.0, 0.0]
    assert model[1][0] - model[0][0] == 5 * 60 * 1000


def test_curves_at_noon():
    model, power, _ = estimate.curves(NOV_24, 0.0, 0.78)
    assert model[144] == [43200000.0, approx(7.42, abs=0.01)]
    assert power[144][1] == approx(0.78 * 7.42, abs=0.01)


def test_curves_energy_ends_at_the_energy_of_the_day():
    _, _, energy = estimate.curves(NOV_24, 0.0, 0.78)
    assert energy[0][1] == 0.0
    assert energy[-1][1] == approx(estimate.day_wh(NOV_24, 0.78), abs=0.01)


def test_curves_without_a_factor_give_only_the_model():
    model, power, energy = estimate.curves(NOV_24, 0.0, None)
    assert len(model) == 288
    assert power == []
    assert energy == []


def test_window_runs_from_before_sunrise_to_after_sunset():
    start, end = estimate.window(NOV_24, 0.0)
    assert start == (466 - 30) * 60 * 1000
    assert end == (991 + 30) * 60 * 1000


def test_energy_text():
    assert estimate.energy_text(36.4) == 'E ≈ 36 Wh'
    assert estimate.energy_text(None) == 'E ≈ ? Wh'


def test_kwh_text():
    assert estimate.kwh_text(36.4) == '= 0.036 kWh for the whole day'
    assert estimate.kwh_text(None) == ''


def test_note_text_with_an_estimate():
    note = estimate.note_text('', 0.78, 5.6, 48.0)
    assert note.startswith('Estimate, not measured.')
    assert '5.6 W' in note
    assert '78 %' in note
    assert '(48 Wh)' in note


def test_note_text_gives_the_reason_when_there_is_no_estimate():
    note = estimate.note_text(estimate.NO_LOAD, None, None, 48.0)
    assert note == ('No estimate: nothing is plugged into the USB socket (current below 0.05 A). '
                    'The dashed line is the clear-sky model for today (48 Wh).')