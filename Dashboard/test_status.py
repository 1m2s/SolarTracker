"""Tests for status.py. Run with `pytest` inside Dashboard/."""

import status


def test_state_of_a_running_tracker():
    assert status.state_text('A', 1, 6, 0) == 'Mode: AUTO · IMU switch: AN · Start sequence: done'
    assert status.state_text('M', 0, 6, 0) == 'Mode: MATLAB · IMU switch: AUS · Start sequence: done'


def test_every_stage_of_the_start_sequence_has_words():
    for stage in range(7):
        assert status.STAGES[stage] != ''
    assert status.state_text('A', 1, 2, 0).endswith('Start sequence: measuring the compass')
    assert status.state_text('A', 1, 4, 0).endswith('Start sequence: turning to South')


def test_state_before_the_right_nano_has_answered():
    assert status.state_text('A', None, 0, 0) == ('Mode: AUTO · IMU switch: ? · '
                                                  'Start sequence: waiting for the right Nano')


def test_missing_and_unknown_values_are_question_marks():
    assert status.state_text(None, None, None, None) == 'Mode: ? · IMU switch: ? · Start sequence: ?'
    assert status.state_text('X', 2, 9, 0) == 'Mode: ? · IMU switch: ? · Start sequence: ?'


def test_a_fault_stops_the_start_sequence():
    assert status.state_text('A', 1, 3, 3).endswith('Start sequence: stopped while aiming at South')
    assert status.state_text('A', None, 0, 1).endswith('Start sequence: stopped while waiting for the right Nano')


def test_a_fault_after_the_start_sequence_leaves_it_done():
    assert status.state_text('A', 1, 6, 2).endswith('Start sequence: done')


def test_a_fault_with_an_unknown_stage_keeps_the_question_mark():
    assert status.state_text('A', 1, None, 3).endswith('Start sequence: ?')


def test_has_fault():
    assert status.has_fault(1)
    assert status.has_fault(9)
    assert not status.has_fault(0)
    assert not status.has_fault(None)


def test_fault_text():
    assert status.fault_text(0) == 'Fault: none'
    assert status.fault_text(None) == 'Fault: ?'
    assert status.fault_text(1) == 'Fault 1: right Nano not answering'
    assert status.fault_text(5) == 'Fault 5: turn to South took too long'
    assert status.fault_text(4) == 'Fault 4: unknown code'
    assert status.fault_text(9) == 'Fault 9: unknown code'


def test_every_fault_of_the_firmware_has_a_name_and_an_action():
    for fault in (1, 2, 3, 5):
        name, action = status.FAULTS[fault]
        assert name != ''
        assert action.endswith('power-cycle.')


def test_no_action_without_a_fault():
    assert status.fault_action(0) == ''
    assert status.fault_action(None) == ''


def test_fault_action_says_what_to_do_and_what_a_power_cycle_is():
    action = status.fault_action(3)
    assert action.startswith('What to do: Check the IMU cable, then power-cycle.\n')
    assert action.endswith(status.POWER_CYCLE)
    assert 'unplug the USB cable' in action
    assert 'Figure 2 pose' in action


def test_fault_action_for_an_unknown_code():
    assert status.fault_action(9).startswith('What to do: Read the messages below, then power-cycle.\n')


def test_moved_text_shows_the_sign_and_how_to_align_again():
    assert status.moved_text(16.84).startswith('⚠ The compass reads +16.8° from power-on.')
    assert status.moved_text(-7.3).startswith('⚠ The compass reads -7.3° from power-on.')
    assert 'Figure 2 pose' in status.moved_text(16.8)
    assert 'roughly south' in status.moved_text(16.8)