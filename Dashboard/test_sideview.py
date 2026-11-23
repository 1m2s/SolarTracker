"""Tests for sideview.py. Run with `pytest` inside Dashboard/."""

import compass
import sideview


def test_draw_gives_one_svg_picture():
    svg = sideview.draw(42.0, 41.6, True)
    assert svg.startswith('<svg')
    assert svg.endswith('</svg>')


def test_panel_is_tilted_by_the_elevation():
    svg = sideview.draw(42.0, 41.6, True)
    assert 'rotate(-42.0 104 168)"><g id="panel"' in svg


def test_upright_panel_is_not_turned():
    svg = sideview.draw(0.0, 41.6, True)
    assert 'rotate(-0.0 104 168)"><g id="panel"' in svg


def test_sun_above_the_horizon_sits_at_its_height_and_is_filled():
    svg = sideview.draw(42.0, 41.6, True)
    assert 'rotate(-41.6 104 168)"><line' in svg
    assert f'id="sun" cx="232" cy="168" r="9" fill="{compass.SUN_YELLOW}"' in svg


def test_sun_below_the_horizon_sits_at_zero_and_is_empty():
    svg = sideview.draw(0.0, -27.8, True)
    assert 'rotate(-0.0 104 168)"><circle id="sun"' in svg
    assert f'id="sun" cx="232" cy="168" r="9" fill="{compass.WHITE}"' in svg


def test_values_are_written_under_the_picture():
    svg = sideview.draw(42.0, 41.6, True)
    assert 'Panel tilted 42° back from upright' in svg
    assert 'Sun 41.6° above the horizon' in svg


def test_sun_below_the_horizon_is_written_without_a_minus():
    svg = sideview.draw(0.0, -27.8, True)
    assert 'Panel tilted 0° back from upright' in svg
    assert 'Sun 27.8° below the horizon' in svg


def test_note_says_the_tilt_is_not_measured():
    svg = sideview.draw(42.0, 41.6, True)
    assert sideview.NOT_MEASURED[0] in svg
    assert sideview.NOT_MEASURED[1] in svg


def test_values_that_are_none_are_not_drawn():
    svg = sideview.draw(None, None, True)
    assert 'id="panel"' not in svg
    assert 'id="sun"' not in svg
    assert 'Panel tilt no value' in svg
    assert 'Sun no value' in svg


def test_without_telemetry_everything_is_grey():
    svg = sideview.draw(42.0, 41.6, False)
    assert 'No telemetry: last known values' in svg
    assert compass.PANEL_BLUE not in svg
    assert compass.SUN_YELLOW not in svg
    assert compass.OLD_GREY in svg


def test_with_telemetry_there_is_no_old_note():
    svg = sideview.draw(42.0, 41.6, True)
    assert 'No telemetry' not in svg