# Compass calibration

How the compass heading in `src/Heading.cpp` works, and how to recalibrate it.

## What the heading means
`tiltHeading()` gives the compass direction of the sensor's x axis. It is the same math as
`getpitch()`, `getroll()` and `getTiltHeading()` in `Photovoltaiknachfuehrung.ino`
(CurrentFirmware), with the same scale 1200 / 4096.

Level board: field along +x → 0°, along −y → 90°, along −x → 180°, along +y → 270°.
`test/logic/test_heading` checks this.

Pitch and roll come from the accelerometer, so a tilted base still gives the right heading.

## Changes compared to CurrentFirmware
- The old code could return 360 instead of 0 (same direction). `fmodf` keeps the result in 0 ≤ heading < 360.
- `asin` of a value just above 1 (sensor noise) gives NaN. `clampToUnitRange` limits it to 1 first.
- The compass runs in **16-bit** mode (`MAG_MEASURE_ONE = 0x11` in `src/Imu.cpp`, was 14-bit).
  Raw counts are 4× bigger and the noise is about ±4 counts. The calibration numbers below are
  for 16-bit; old 14-bit numbers must not be reused.

## Calibration (`magCalibrated`)
1. **Centre (hard iron):** turning the tracker should draw a circle around 0. The real circle
   sits at `MX_CENTRE`, `MY_CENTRE`, so that is subtracted.
2. **Oval (soft iron):** iron near the sensor (probably the stepper motor) squashes the circle
   into an oval. `SOFT_XX`, `SOFT_XY`, `SOFT_YY` stretch it back. Without this step, headings in
   the middle of a turn are off by up to about 12°.

All five numbers come from `fit_compass.py` in this folder, from three exact 180° turns
(see `test/README.md` for the commands). The cables only allow half a turn, so a full circle
is not possible. Run it again if the tracker gets new parts near the sensor or the readings
look wrong.

Calibration only removes iron that **turns with the sensor**. Iron in the room (building
steel, furniture, radiators) is not removed, so calibrate **outdoors**.

### Result (10 Nov 2026, lawn away from buildings, after dark)
Board levelled with a phone app, tape arrow on the base for the exact 180° turns, laptop at
least 1 m away on a USB extension, phone in the pocket during the runs.

| Run | Start (raw x, y) | End (raw x, y) |
|---|---|---|
| `run1.txt` | (−30.7, 90.6) | (66.7, 276.9) |
| `run2.txt` | (64.5, 278.1) | (−25.0, 87.5) |
| `run3.txt` | (65.5, 278.2) | (−22.0, 85.5) |

`fit_compass.py`: all checks passed.

| Check | Result |
|---|---|
| Centre (raw) | (19.83, 182.80) |
| Oval radii | 157.9 and 104.6 (ratio 1.51) |
| Check 1: each run reads as a 180° turn | 178.5°, 179.9°, 178.4° |
| Check 2: radius spread | 30 % before → 7 % after the oval correction |
| Fairness check (leave one out) | 177.8°, 179.9°, 177.7° |

Numbers in `include/config/Calibration.h`: `MX_CENTRE = 5.81`, `MY_CENTRE = 53.55`,
`SOFT_XX = 0.9373`, `SOFT_XY = 0.1896`, `SOFT_YY = 1.1053`.

## Mounting offset (`IMU_TO_PANEL_DEG`)
The heading is the direction of the **sensor's x axis**. The firmware needs the direction the
**panel faces** in the start pose of Figure 2 (Abbildung 2 in the Bedienungsanleitung):
panel bar across the base arrow, solar-cell face pointing **opposite** to the arrow.

$$h_{\text{panel}} = h_{\text{sensor}} + \text{IMU\\_TO\\_PANEL\\_DEG} + \text{MAG\\_DECLINATION\\_DEG}$$

`MAG_DECLINATION_DEG = 3.9` turns magnetic north into true north (the sun math uses true north).

**Result (10 Nov 2026): `IMU_TO_PANEL_DEG = 172`.** In `include/config/Calibration.h`, used by
`panelHeading` in `src/Correction.cpp`.

### How it was measured
1. Mast in the start pose of Figure 2, not touched during the whole measurement.
2. Phone compass set to **magnetic** north (Use True North off), because the sensor also
   gives magnetic north.
3. Phone flat on a plank on the grass, top edge pointing **along the base arrow**, at least
   1 m away from the tracker → direction of the arrow $d$.
4. Sensor heading $h_{\text{sensor}}$: `setup/test_compass` (average of 40 readings) or
   `setup/test_panel_offset` (2 s settle, 20 s of reads averaged as raw values, the same
   method as the start sequence; needs `-v`).
5. Arrow offset $= d - h_{\text{sensor}}$.
6. Whole tracker turned by about 180°, steps 3 to 5 again.

### Results

| Position | Phone (arrow) | Sensor | Source | Arrow offset | Log |
|---|---|---|---|---|---|
| 1 | 285° | 295° | `test_compass` (40 readings, ±2°) | −10.0° | not logged |
| 2 | 115° | 120.6° | `test_panel_offset` (429 good reads, field xy 38.1) | −5.6° | `offset_s.txt` |

### Why the average of the two positions, and why +180°
If the calibration centre is a little off, it adds an error $e$ to the heading. Turning the
tracker by 180° flips the sign of that error:

$$
\begin{cases}
\text{offset}_1 = \text{true offset} + e \\
\text{offset}_2 = \text{true offset} - e
\end{cases}
\quad\Rightarrow\quad
\text{true offset} = \frac{-10.0^\circ + (-5.6^\circ)}{2} = -7.8^\circ, \qquad e \approx 2.2^\circ
$$

The phone measured the **arrow**, but the panel faces **opposite** to the arrow, so:

$$\text{IMU\\_TO\\_PANEL\\_DEG} = -7.8^\circ + 180^\circ = 172.2^\circ \approx 172^\circ$$

So one position alone is not enough; always measure two positions 180° apart and take the average.

### Checked on the tracker (12 Nov 2026, indoors)
Start sequence: `h=35.3  panel=211.2`. The panel really faced about 210° by eye, and it
turned about 31° anticlockwise to South. Sun angle in the same run: 200.2° / 22.3°, which
matches a hand calculation for Weingarten.

## Other checks (10 Nov 2026, outdoors)
- **90° check** (`test_compass`, base placed by eye at 0°, about 90° and 180°): headings 121°,
  204°, 295° → steps 83° and 91°. Fits for positions placed by eye. Noise ±1–2°.
- **Tilt check** (pen under one side, about 2° pitch): raw values changed, heading stayed at
  121–122° → tilt compensation works for small tilts. Not tested: the other axis, tilts above
  about 3°.
- **Field strength** (calibrated units): outdoors xy ≈ 38, raw z ≈ +116. This is the reference
  for a clean spot.

## Lessons
- **Outdoors only.** Indoors (10 Nov, a room) the field was xy 26.8 instead of 38 and raw z
  −16 instead of +116, so the building bends the field. Readings there can be tens of
  degrees off. `test_panel_offset -v` prints `field strength xy` and works as a meter for
  finding a clean spot.
- **Save `Calibration.h` before flashing.** On 10 Nov the first 90° check ran with the old
  numbers because the file was not saved in VS Code.
- Laptops at least 1 m away; your own MacBook alone moved the heading by about 8° (4 Nov).
- After turning the base, check the IMU wires: on 4 Nov one run lost the compass (`0x0C` did
  not answer).

## History
- **4 Nov 2026 (lab, 14-bit):** first calibration on an empty lab table, arrow offset −14°
  (two positions −22° and −7°, error ±8°). The field there was only about 19 counts (14-bit),
  so the building was already bending it. Replaced by the outdoor calibration.
- **10 Nov 2026:** the panel offset was first entered as 82°, computed as if the phone had
  pointed along the panel edge ($d + 90^\circ$). It had pointed along the arrow, so this was
  90° off and made the panel turn the wrong way at startup. Fixed to 172° on 12 Nov.

## Not checked yet
- `MZ_CENTRE` is not measured: −28 is the 2020 value (−7) × 4 for 16-bit. A flat turn cannot
  measure it, and on a level base z hardly changes the heading. Place the tracker on roughly
  flat ground.
- A 270° position (the cables only allow half a turn).
- The phone compass and lining up the plank by eye are now the weakest part (a few degrees).
  A better reference would be a building wall whose direction is read from a map (that gives
  true north, so subtract the declination first).