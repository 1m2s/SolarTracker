"""Tests for topview.py. Run with `pytest` inside Dashboard/."""

import compass
import topview


def test_draw_gives_one_svg_picture():
    svg = topview.draw(179.5, 277.5, 270.0, -27.8, True)
    assert svg.startswith('<svg')
    assert svg.endswith('</svg>')


def test_marks_are_turned_to_their_directions():
    svg = topview.draw(179.5, 277.5, 270.0, -27.8, True)
    assert 'rotate(179.5 160 160)"><g id="panel"' in svg
    assert 'rotate(277.5 160 160)"><g id="base"' in svg
    assert 'rotate(270.0 160 160)"><circle id="sun"' in svg


def test_base_is_drawn_under_the_panel():
    svg = topview.draw(179.5, 277.5, 270.0, -27.8, True)
    assert svg.index('id="base"') < svg.index('id="panel"')


def test_values_are_written_under_the_picture():
    svg = topview.draw(179.5, 277.5, 270.0, -27.8, True)
    assert 'Panel faces 179.5°' in svg
    assert 'Base: board side faces 277.5°' in svg
    assert 'Sun 270.0°' in svg


def test_sun_above_the_horizon_is_filled():
    up = topview.draw(None, None, 135.0, 23.4, True)
    down = topview.draw(None, None, 135.0, -5.0, True)
    assert f'id="sun" cx="160" cy="40" r="9" fill="{compass.SUN_YELLOW}"' in up
    assert f'id="sun" cx="160" cy="40" r="9" fill="{compass.WHITE}"' in down


def test_note_says_the_turn_is_not_measured():
    svg = topview.draw(179.5, 277.5, 270.0, -27.8, True)
    assert topview.NOT_MEASURED[0] in svg
    assert topview.NOT_MEASURED[1] in svg


def test_values_that_are_none_are_not_drawn():
    svg = topview.draw(None, None, None, None, True)
    assert 'id="panel"' not in svg
    assert 'id="base"' not in svg
    assert 'id="sun"' not in svg
    assert 'Panel faces no value' in svg
    assert 'Base: board side faces no value' in svg
    assert 'Sun no value' in svg


def test_panel_is_drawn_without_the_base_when_the_power_on_direction_is_missing():
    svg = topview.draw(180.0, None, 135.0, 23.4, True)
    assert 'rotate(180.0 160 160)"><g id="panel"' in svg
    assert 'id="base"' not in svg


def test_without_telemetry_everything_is_grey():
    svg = topview.draw(179.5, 277.5, 135.0, 23.4, False)
    assert 'No telemetry: last known values' in svg
    assert compass.PANEL_BLUE not in svg
    assert compass.SUN_YELLOW not in svg
    assert compass.OLD_GREY in svg


def test_with_telemetry_there_is_no_old_note():
    svg = topview.draw(179.5, 277.5, 135.0, 23.4, True)
    assert 'No telemetry' not in svg