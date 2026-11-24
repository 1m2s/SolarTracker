"""Tests for telemetry.py. Run with `pytest` inside Dashboard/."""

import telemetry

FULL_LINE = ('T:MODE=A ST=6 FLT=0 ERR=0 IMU=1 MOV=0 SUN=257.6 SEL=-13.9 PAN=304.6 HDG=304.8 '
             'AZ=56 EL=0 A0=2712 A1=19')


def test_full_line_gives_all_values():
    assert telemetry.parse(FULL_LINE) == {
        'MODE': 'A', 'ST': 6, 'FLT': 0, 'ERR': 0, 'IMU': 1, 'MOV': 0,
        'SUN': 257.6, 'SEL': -13.9, 'PAN': 304.6, 'HDG': 304.8,
        'AZ': 56.0, 'EL': 0.0, 'A0': 2712, 'A1': 19,
    }

def test_line_without_the_sums_leaves_them_out():
    values = telemetry.parse('T:MODE=A ST=6 FLT=0 ERR=1 IMU=1 MOV=0 AZ=56 EL=0')
    assert values['ERR'] == 1
    assert 'A0' not in values
    assert 'A1' not in values


def test_sums_are_whole_numbers():
    assert telemetry.parse('T:A0=0 A1=16368') == {'A0': 0, 'A1': 16368}
    assert telemetry.parse('T:A0=27.5 A1=x ERR=yes') == {}
def test_field_strength_keys_are_numbers():
    assert telemetry.parse('T:HDG=284.1 FXY=38.1 FZ=62.0') == {'HDG': 284.1, 'FXY': 38.1, 'FZ': 62.0}
    assert telemetry.parse('T:FXY=nan FZ=x') == {}
def test_other_lines_are_not_telemetry():
    assert telemetry.parse('PARK south') is None
    assert telemetry.parse('AUTO  az=180.0000  el=0.0000  azByte=90  elByte=0') is None
    assert telemetry.parse('') is None
    assert telemetry.parse(' T:MODE=A') is None


def test_prefix_alone_gives_no_values():
    assert telemetry.parse('T:') == {}


def test_missing_keys_are_left_out():
    assert telemetry.parse('T:MODE=M ST=0 FLT=0 AZ=180 EL=0') == {
        'MODE': 'M', 'ST': 0, 'FLT': 0, 'AZ': 180.0, 'EL': 0.0,
    }


def test_unknown_keys_are_ignored():
    assert telemetry.parse('T:ST=6 PAZ=512 XYZ=abc EL=40') == {'ST': 6, 'EL': 40.0}


def test_values_that_are_not_numbers_are_dropped():
    assert telemetry.parse('T:SUN=nan SEL=inf PAN=ovf HDG=-inf AZ=56') == {'AZ': 56.0}
    assert telemetry.parse('T:ST=x FLT=1.5 IMU= MOV=0') == {'MOV': 0}


def test_unknown_mode_is_dropped():
    assert telemetry.parse('T:MODE=X ST=6') == {'ST': 6}


def test_pairs_without_equals_are_ignored():
    assert telemetry.parse('T:ST EL=40 =5') == {'EL': 40.0}


def test_extra_spaces_do_not_matter():
    assert telemetry.parse('T:ST=6   EL=40 ') == {'ST': 6, 'EL': 40.0}


def test_the_last_value_wins_when_a_key_is_repeated():
    assert telemetry.parse('T:EL=10 EL=40') == {'EL': 40.0}


def test_update_stores_the_values():
    telemetry.clear()
    assert telemetry.update(FULL_LINE, 100.0) is True
    assert telemetry.value('MODE', 100.5) == 'A'
    assert telemetry.value('HDG', 100.5) == 304.8


def test_update_ignores_other_lines():
    telemetry.clear()
    assert telemetry.update('PARK south', 100.0) is False
    assert telemetry.value('MODE', 100.0) is None


def test_value_is_none_before_anything_arrived():
    telemetry.clear()
    assert telemetry.value('HDG', 100.0) is None
    assert telemetry.last_value('HDG') is None


def test_value_gets_old():
    telemetry.clear()
    telemetry.update('T:HDG=304.8', 100.0)
    assert telemetry.value('HDG', 100.0 + telemetry.MAX_AGE_S) == 304.8
    assert telemetry.value('HDG', 100.1 + telemetry.MAX_AGE_S) is None
    assert telemetry.last_value('HDG') == 304.8


def test_a_key_missing_from_a_newer_line_keeps_its_old_time():
    telemetry.clear()
    telemetry.update('T:HDG=304.8 AZ=56', 100.0)
    telemetry.update('T:AZ=58', 104.0)
    assert telemetry.value('AZ', 104.0) == 58.0
    assert telemetry.value('HDG', 104.0) is None
    assert telemetry.last_value('HDG') == 304.8


def test_a_newer_value_replaces_the_old_one():
    telemetry.clear()
    telemetry.update('T:HDG=304.8', 100.0)
    telemetry.update('T:HDG=305.1', 101.0)
    assert telemetry.value('HDG', 101.0) == 305.1


def test_clear_forgets_everything():
    telemetry.update(FULL_LINE, 100.0)
    telemetry.clear()
    assert telemetry.last_value('MODE') is None