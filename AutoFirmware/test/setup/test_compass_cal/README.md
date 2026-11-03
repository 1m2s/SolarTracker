# Compass calibration

How the compass heading in `src/Heading.cpp` works, and how to recalibrate it.

## What the heading means
`tiltHeading()` gives the compass direction of the sensor's x axis. It is the same math as
`getpitch()`, `getroll()` and `getTiltHeading()` in `Photovoltaiknachfuehrung.ino`
(CurrentFirmware), with the same scale 1200 / 4096.

Level board: field along +x → 0°, along −y → 90°, along −x → 180°, along +y → 270°.
`test/logic/test_heading` checks this.

Pitch and roll come from the accelerometer, so a tilted base still gives the right heading.

## Two changes compared to CurrentFirmware
- The old code could return 360 instead of 0 (same direction). `fmodf` keeps the result in 0 ≤ heading < 360.
- `asin` of a value just above 1 (sensor noise) gives NaN. `clampToUnitRange` limits it to 1 first.

## Calibration (`magCalibrated`)
1. **Centre:** turning the tracker should draw a circle around 0. The real circle sits at
   `MX_CENTRE`, `MY_CENTRE`, so that is subtracted.
2. **Oval:** iron near the sensor (probably the stepper motor) squashes the circle into an
   oval (about 19 : 13). `SOFT_XX`, `SOFT_XY`, `SOFT_YY` stretch it back. Without this step,
   headings in the middle of a turn are off by up to about 11°.

All five numbers come from `fit_compass.py` in this folder, from three exact 180° turns
(see `test/README.md` for the commands). Run it again if the tracker gets new parts near
the sensor or the readings look wrong.

## Not checked yet
- A 90° position: `fit_compass.py` can only check the start and end of a turn.
  Next lab check: `setup/test_compass` at 0°, 90° and 180°.
- `MZ_CENTRE` is still the 2020 value. A flat turn cannot measure it, and on a level base
  z hardly changes the heading.