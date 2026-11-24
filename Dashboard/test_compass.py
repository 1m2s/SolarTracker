"""Tests for compass.py. Run with `pytest` inside Dashboard/."""

import compass


def test_draw_gives_one_svg_picture():
    svg = compass.draw(180.6, 261.8, -17.9, 304.6, 304.4, True)
    assert svg.startswith('<svg')
    assert svg.endswith('</svg>')


def test_marks_are_turned_to_their_directions():
    svg = compass.draw(180.6, 261.8, -17.9, 304.6, 304.4, True)
    assert 'rotate(180.6 160 160)"><g id="panel"' in svg
    assert 'rotate(261.8 160 160)"><circle id="sun"' in svg
    assert 'rotate(304.6 160 160)"><line id="power-on"' in svg
    assert 'rotate(304.4 160 160)"><polygon id="heading"' in svg


def test_values_are_written_under_the_compass():
    svg = compass.draw(180.6, 261.8, -17.9, 304.6, 304.4, True)
    assert 'Panel faces 180.6°' in svg
    assert 'Sun 261.8°, 17.9° below the horizon' in svg
    assert 'Compass now 304.4° (-0.2° from power-on)' in svg
    assert 'At power-on 304.6°' in svg


def test_sun_above_the_horizon_is_filled():
    up = compass.draw(None, 135.0, 23.4, None, None, True)
    down = compass.draw(None, 135.0, -5.0, None, None, True)
    assert 'Sun 135.0°, 23.4° above the horizon' in up
    assert f'id="sun" cx="160" cy="40" r="9" fill="{compass.SUN_YELLOW}"' in up
    assert f'id="sun" cx="160" cy="40" r="9" fill="{compass.WHITE}"' in down


def test_values_that_are_none_are_not_drawn():
    svg = compass.draw(None, None, None, None, None, True)
    assert 'id="panel"' not in svg
    assert 'id="sun"' not in svg
    assert 'id="power-on"' not in svg
    assert 'id="heading"' not in svg
    assert 'Panel faces no value' in svg
    assert 'Compass now no value' in svg


def test_without_telemetry_everything_is_grey():
    svg = compass.draw(180.6, 261.8, 20.0, 304.6, 304.4, False)
    assert 'No telemetry: last known values' in svg
    assert compass.PANEL_BLUE not in svg
    assert compass.SUN_YELLOW not in svg
    assert compass.OLD_GREY in svg


def test_with_telemetry_there_is_no_old_note():
    svg = compass.draw(180.6, 261.8, 20.0, 304.6, 304.4, True)
    assert 'No telemetry' not in svg