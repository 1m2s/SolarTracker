# SolarTracker

This project extends an existing RWU solar tracker that was controlled through MATLAB with the MATLAB Support Package for Arduino Hardware. The left Arduino Nano now runs its own firmware: it can track the sun on its own (AUTO, no computer needed) or take the sun angle from MATLAB over USB (MATLAB mode). With the IMU switch on AN, a compass corrects the panel when the base is turned. Firmware is developed with VS Code and PlatformIO.

**How to use the tracker:** [`AutoFirmware/HowToOperate.md`](AutoFirmware/HowToOperate.md)

## Folders

| Folder | What it is | Runs on |
|---|---|---|
| [`AutoFirmware/`](AutoFirmware/) | New firmware: AUTO/MATLAB modes, sun position (port of the MATLAB code), IMU (compass) mode, status LED | Left Nano (`A_MATLAB`) |
| [`MotorFirmware/`](MotorFirmware/) | Moves the panel (stepper + servo), shows voltage and power, reports the IMU switch and "motor moving" | Right Nano (`A_AIDE`), I2C address `0x19` |
| [`MatlabSerial/`](MatlabSerial/) | MATLAB script for MATLAB mode (sends the sun angle as text over USB) | Mac |
| [`CurrentFirmware/`](CurrentFirmware/) | The original firmware and MATLAB scripts, unchanged (reference and rollback) | – |
| [`images/`](images/) | Pictures used in the docs | – |

## More

- Tests, commands and the checks before every commit: [`AutoFirmware/test/README.md`](AutoFirmware/test/README.md)
- Which MATLAB file each C++ function comes from: [`AutoFirmware/MATLAB_MAPPING.md`](AutoFirmware/MATLAB_MAPPING.md)
- Compass calibration and mounting offset: [`AutoFirmware/test/setup/test_compass_cal/README.md`](AutoFirmware/test/setup/test_compass_cal/README.md)
- Right Nano tests: [`MotorFirmware/test/README.md`](MotorFirmware/test/README.md)