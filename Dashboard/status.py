"""Tracker status in words.

Turns the codes of the telemetry line (MODE, IMU, ST, FLT) into text for the
page. The fault names and the actions follow AutoFirmware/HowToOperate.md.
"""

UNKNOWN = '?'
NO_FAULT = 0
STAGE_DONE = 6

MODES = {'A': 'AUTO', 'M': 'MATLAB'}
IMU_SWITCH = {1: 'AN', 0: 'AUS'}
STAGES = {
    0: 'waiting for the right Nano',
    1: 'settling',
    2: 'measuring the compass',
    3: 'aiming at South',
    4: 'turning to South',
    5: 'holding at South',
    6: 'done',
}
FAULTS = {
    1: ('right Nano not answering', 'Check the right Nano and the I2C wires, then power-cycle.'),
    2: ('IMU switch moved', 'Do the alignment for the new switch position, then power-cycle.'),
    3: ('no usable compass reading', 'Check the IMU cable, then power-cycle.'),
    4: ('panel faces north', 'Turn the whole tracker around, then power-cycle.'),
    5: ('turn to South took too long', 'Check the motor and the right Nano, then power-cycle.'),
}
UNKNOWN_FAULT = ('unknown code', 'Read the messages below, then power-cycle.')

POWER_CYCLE = ('Power-cycle: power OFF and unplug the USB cable, do the alignment, then start again '
               '(bridge.py too: it stops when the cable is unplugged).\n'
               'Alignment with IMU AN: panel upright in the Figure 2 pose, tracker placed roughly south.\n'
               'Alignment with IMU AUS: panel upright, base turned so the panel faces South.')
ALIGN_AGAIN = ('To align again: power OFF and unplug the USB cable, set the panel upright in the '
               'Figure 2 pose, place the tracker roughly south, then start again.')


def has_fault(fault: int | None) -> bool:
    """True if the fault code is known and is not 0."""
    return fault is not None and fault != NO_FAULT


def state_text(mode: str | None, imu: int | None, stage: int | None, fault: int | None) -> str:
    """Give back mode, IMU switch and start sequence in words, on one line.

    A value that is None or not in the tables is shown as a question mark.
    After a fault the start sequence does not go on, so the stage it stopped
    in is shown as "stopped while ...".
    """
    stage_words = STAGES.get(stage, UNKNOWN)
    if has_fault(fault) and stage in STAGES and stage != STAGE_DONE:
        stage_words = f'stopped while {stage_words}'
    return (f'Mode: {MODES.get(mode, UNKNOWN)} · IMU switch: {IMU_SWITCH.get(imu, UNKNOWN)} · '
            f'Start sequence: {stage_words}')


def fault_text(fault: int | None) -> str:
    """Give back the fault in words, for example `Fault 3: no usable compass reading`."""
    if fault is None:
        return f'Fault: {UNKNOWN}'
    if fault == NO_FAULT:
        return 'Fault: none'
    name, _ = FAULTS.get(fault, UNKNOWN_FAULT)
    return f'Fault {fault}: {name}'


def fault_action(fault: int | None) -> str:
    """Give back what to do about the fault, or an empty text if there is no fault."""
    if not has_fault(fault):
        return ''
    _, action = FAULTS.get(fault, UNKNOWN_FAULT)
    return f'What to do: {action}\n{POWER_CYCLE}'


def moved_text(moved_by: float) -> str:
    """Give back the warning for a compass reading that is `moved_by` degrees from power-on."""
    return f'⚠ The compass reads {moved_by:+.1f}° from power-on. Was the tracker moved? {ALIGN_AGAIN}'