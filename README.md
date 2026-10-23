# SolarTracker

Continuation of the existing RWU photovoltaic tracker. Initial work focuses on reproducing the original operation before adding autonomous control.

## Verified baseline — 22–23 October 2026

- Desktop MATLAB on a Mac communicates with the left Arduino Nano V3 (`A_MATLAB`) over USB.
- MATLAB Support Package for Arduino Hardware installed; Nano3 selected with I2C support.
- USB serial port observed: `/dev/cu.usbserial-AC013FMP` (machine-specific).
- The left Arduino bridges MATLAB commands to the right controller (`A_AIDE`) over I2C, address `0x19`.
- `AusgabeSonnenstand.m` is the main script. `SonnenStand.m` and its helper functions calculate solar azimuth and elevation from location, date and time.
- Location configured in the supplied script: longitude 9.66°, latitude 47.81°.
- Register 254 receives azimuth divided by two as uint8; register 255 receives elevation as uint8. The script updates every 30 seconds and uses time only to minute resolution.
- Operation without IMU was reported working after manual south alignment. Example calculated targets: azimuth 243.3049°, elevation 8.5946°.
- Operation with IMU was subsequently reported working. Example calculated targets: azimuth 175.5472°, elevation 30.9212°.
- These observations establish functional operation; angular accuracy has not been quantitatively measured.

## Hardware roles

| Component | Role |
| --- | --- |
| Left Nano (`A_MATLAB`) | MATLAB USB interface and I2C bridge |
| Right Nano (`A_AIDE`) | Existing tracker firmware, motor control and sensor handling |
| Stepper and L298N | Azimuth rotation |
| Servo and linkage | Elevation adjustment |
| IMU magnetometer | Base heading reference; not a sunlight sensor |
| Power bank | Motor power through the existing power-input connection |

The existing IMU switch selects heading-reference behavior. It does not select MATLAB versus autonomous operation.

## Operating reference

Follow `Bedienungsanleitung.pdf`, particularly its switch photograph and Figure 2. With IMU off, start with the panel upright and its solar-cell face toward south. With IMU on, start upright and align the panel relative to the base as shown in Figure 2; the entire base need not face south. Keep the base level.

For changing modes, the manual specifies: power off, select IMU mode, set the starting position, hold RESET on the right Arduino, power on, then release RESET. Connect the computer to the left Arduino and run the MATLAB script with the correct port. Stop MATLAB with Control+C; switch physical power off when finished. Control+C does not guarantee that a motor command already received has stopped.

## Connection diagnostics

Run in the MATLAB Command Window:

```matlab
clear slave ar
ar = arduino('/dev/cu.usbserial-AC013FMP','Nano3','Libraries','I2C');
scanI2CBus(ar)
```

`clear` removes MATLAB objects; it does not erase Arduino firmware. A successful constructor establishes USB communication. An I2C scan containing `0x19` confirms that a device responds at the expected second-controller address; it does not validate motor behavior.

Observed scans included `0x19`, `0x68`, `0x77`, and sometimes `0x0C`. Exact identities of the additional addresses have not been independently verified.

Intermittent `Unable to receive data from the target hardware` errors occurred during I2C scans and at:

```matlab
slave = device(ar,'I2CAddress','0x19','bitrate',100000);
```

Communication recovered after restarting/reconnecting the setup. Root cause remains unconfirmed; power, bus state and startup behavior need further investigation. Do not record this as a proven power-bank fault.

The full MATLAB script creates its own connection; separate diagnostic commands are not required for every normal startup.

## Proposed engineering strategy — not implemented

Keep one firmware project with two target sources and one shared motor-control path:

- MATLAB source: existing received azimuth/elevation targets.
- AUTO source: locally generated azimuth/elevation targets.
- Mode selection: only the selected source may supply targets to the motors.
- Shared motor routines: retain the existing mechanism-specific positioning logic.

First decide what AUTO means. Calculating sun position locally requires a reliable time/date source and configured location. Optical tracking requires light sensors, suitable geometry and a tracking algorithm. The IMU alone does not supply either sun position or time.

### Stages

1. Preserve the original firmware and reproduce its behavior in VS Code / PlatformIO on the right Nano. Inspect the actual `.ino`, dependencies, pin assignments and bootloader before choosing configuration. Keep the left MATLAB bridge intact.
2. Separate target selection from existing motor routines while retaining working MATLAB operation. Introduce an explicit MATLAB/AUTO mode internally, initially selected in software.
3. Implement and test the chosen autonomous target source through the same motor routines.
4. Add a breadboard interface if desired: MATLAB button, AUTO button, and one active-mode LED per mode. Check available pins and electrical connections first. Debounce buttons and use LED resistors. LEDs should report the actual selected mode.
5. Test mode changes during motion. Define bounded movement interruption, handling of stale MATLAB commands, initial AUTO target validity, and a documented startup mode.

Design the mode interface early, but introduce hardware and firmware changes in separate testable steps. Two modes should not become two separate copies of the motor-control code.

## Next required source

Inspect `Photovoltaiknachfuehrung.ino` (identified by the original assembly guide as the right Arduino's sketch), plus its required libraries. That source has not yet been reviewed in this commissioning session. Pin allocation, exact IMU behavior, movement blocking and mode-switch implementation must be based on it.
