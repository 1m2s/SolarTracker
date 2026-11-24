"""Tests for history.py. Run with `pytest` inside Dashboard/."""

import pytest
from pytest import approx

import history


@pytest.fixture(autouse=True)
def empty_history():
    history.clear()


def test_no_points_gives_three_empty_lists():
    assert history.series(1000.0) == ([], [], [])


def test_one_point_gives_volts_amps_and_watts():
    history.add(1000.0, 5705, 1184)
    volts, amps, watts = history.series(1000.0)
    assert volts == [[1000000.0, 9.66]]
    assert amps == [[1000000.0, 1.085]]
    assert watts == [[1000000.0, approx(10.49, abs=0.01)]]


def test_points_stay_in_the_order_they_arrived():
    history.add(1000.0, 2629, 19)
    history.add(1001.0, 5705, 19)
    volts, _, _ = history.series(1001.0)
    assert volts == [[1000000.0, 4.45], [1001000.0, 9.66]]


def test_point_older_than_the_window_is_dropped():
    history.add(1000.0, 2629, 19)
    history.add(1001.0, 5705, 19)
    volts, amps, watts = history.series(1000.0 + history.WINDOW_S + 0.5)
    assert volts == [[1001000.0, 9.66]]
    assert len(amps) == 1
    assert len(watts) == 1


def test_point_exactly_as_old_as_the_window_is_kept():
    history.add(1000.0, 2629, 19)
    volts, _, _ = history.series(1000.0 + history.WINDOW_S)
    assert len(volts) == 1


def test_adding_a_point_drops_the_old_ones():
    history.add(1000.0, 2629, 19)
    history.add(1000.0 + history.WINDOW_S + 1.0, 5705, 19)
    assert len(history.points) == 2
    history.add(1000.0 + history.WINDOW_S + 2.0, 5705, 19)
    volts, _, _ = history.series(1000.0 + history.WINDOW_S + 2.0)
    assert [value for _, value in volts] == [None, 9.66, 9.66]


def test_line_without_the_sums_gives_an_empty_point():
    history.add(1000.0, None, None)
    assert history.series(1000.0) == ([[1000000.0, None]], [[1000000.0, None]], [[1000000.0, None]])


def test_sum_outside_the_range_gives_an_empty_point():
    history.add(1000.0, 16369, 19)
    volts, amps, watts = history.series(1000.0)
    assert volts == [[1000000.0, None]]
    assert amps == [[1000000.0, 0.017]]
    assert watts == [[1000000.0, None]]


def test_pause_longer_than_the_gap_breaks_the_line():
    history.add(1000.0, 2629, 19)
    history.add(1000.0 + history.GAP_S + 0.1, 2629, 19)
    volts, _, _ = history.series(1010.0)
    assert [value for _, value in volts] == [4.45, None, 4.45]


def test_pause_as_long_as_the_gap_does_not_break_the_line():
    history.add(1000.0, 2629, 19)
    history.add(1000.0 + history.GAP_S, 2629, 19)
    volts, _, _ = history.series(1010.0)
    assert [value for _, value in volts] == [4.45, 4.45]


def test_window_is_ten_minutes_ending_now():
    assert history.window(1000.0) == (400000.0, 1000000.0)