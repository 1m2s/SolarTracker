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

## Mounting offset (`IMU_TO_ARROW_DEG`)
The heading is the direction of the **sensor's x axis**. The IMU mode needs the direction of
the **base arrow** (Abbildung 2 in the Bedienungsanleitung: straight out from the long side of
the base, away from the electronics side). If the sensor sits at an angle to the arrow, every
heading is off by that angle:

$$h_{\text{arrow}} = h_{\text{sensor}} + \text{IMU\\_TO\\_ARROW\\_DEG}$$

**Result (4 Nov 2026): `IMU_TO_ARROW_DEG = -14`.** Not in the code yet; it goes in with the IMU mode.

### How it was measured
1. Phone compass set to **magnetic** north (the sensor also gives magnetic north; the
   declination is a separate constant).
2. Phone flat, top edge pointing along the base arrow, about 1 m away from the tracker
   → direction of the arrow.
3. `setup/test_compass` for 20 s without touching anything → average of the 40 headings.
4. Offset = phone − sensor.
5. Base turned by about 180°, steps 2 to 4 again.

### Results (empty table, all laptops at least 1 m away)

| Position | Phone (arrow) | Sensor (average of 40) | Spread | Offset | Log |
|---|---|---|---|---|---|
| SE | 115° | 137° | ±2° | −22° | `offset_se.txt` |
| NW | 304° | 311° | ±2° | −7° | `offset_nw.txt` |

### Why the average of the two positions
If the calibration centre is a little off, it adds an error $e$ to the heading. Turning the
base by 180° flips the sign of that error:

$$
\begin{cases}
\text{offset}_{\text{SE}} = \text{true offset} + e \\
\text{offset}_{\text{NW}} = \text{true offset} - e
\end{cases}
\quad\Rightarrow\quad
\text{true offset} = \frac{-22^\circ + (-7^\circ)}{2} \approx -14^\circ, \qquad e \approx 8^\circ
$$

So one position alone is not enough; always measure two positions 180° apart and take the average.

### Why the first runs were thrown away
The first runs were on a lab desk with five laptops around it. Offsets came out as −14°, −65°,
+8° and +16°, with ±5° noise.
- The own MacBook alone moved the heading by about 8° (far vs. close).
- In one position the field was almost twice as strong as usual (32 instead of about 19 raw
  counts from the calibration centre), so something magnetic was close to the sensor.
- One run lost the compass (`0x0C` did not answer) right after the base was turned: loose
  contact, check the IMU wires after turning.

Lesson: measure on an empty table, other laptops at least 1 m away, own laptop as far as the
USB cable allows.

### Accuracy
The phone compass is now the weakest part (a few degrees). Re-check later with a better
reference, for example a building wall whose direction is read from a map (that gives true
north, so subtract the declination first).

## Not checked yet
- A 90° position: `fit_compass.py` can only check the start and end of a turn.
  Next lab check: `setup/test_compass` at 0°, 90° and 180°.
- `MZ_CENTRE` is still the 2020 value. A flat turn cannot measure it, and on a level base
  z hardly changes the heading. Note: raw z was about −3 on the lab desk and about +27 on the
  empty table (4 Nov), so z depends a lot on what is under and around the tracker.
- The centre is a little off: the midpoint of the two offset runs is (10, 52) raw counts, the
  fit gave (7, 48) (the same point as `MX_CENTRE`, `MY_CENTRE` in raw counts). That costs
  about ±8° in some directions. Recalibrate on an empty table with the laptops away.