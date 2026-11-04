# How to operate (AutoFirmware)

For the tracker with **AutoFirmware** on the left Nano (`A_MATLAB`) and **MotorFirmware** on the right Nano.
The original way with MATLAB and `arduino()` is in [`../CurrentFirmware/HowToOperate.md`](../CurrentFirmware/HowToOperate.md); don't use it with this firmware (see MATLAB mode below).

## Two switches

| Switch | Where | Positions | What it does |
|---|---|---|---|
| Mode switch | left Nano, D2 | **AUTO** (open) / **MATLAB** (to GND) | Where the sun angle comes from: the Nano's own clock and sun calculation, or MATLAB over USB. Can be flipped while running. |
| IMU switch | PCB (`S_PROGRAMM`, right Nano D7) | **AN** / **AUS** | AUS: the base must face South. AN: the compass measures where the base points and the panel is corrected. **Only change it with the power off.** |

Both combinations work: AUTO or MATLAB, each with IMU AN or AUS.

## Before power-on: alignment

The right Nano assumes the panel faces its start direction and stands upright when the power comes on.

### IMU AUS

1. Power OFF. IMU switch on **AUS**.
2. Set the panel upright using the servo linkage.
3. Turn the whole base so the panel faces **South**.
4. Keep the base level.

### IMU AN

1. Power OFF. IMU switch on **AN**.
2. Set the panel upright using the servo linkage.
3. Turn the panel so it points along the **base arrow** (Figure 2, page 2 of `Bedienungsanleitung.pdf`): straight out from the long side of the base, away from the electronics side.

   ![Panel starting position for IMU mode](../images/figure2.png)

4. Keep the base level. The base can point in any direction.
5. Keep magnets, metal and laptops at least 1 m away from the IMU. A MacBook close by moves the compass reading by several degrees.

## Start

1. Do the alignment for the IMU switch position.
2. Set the mode switch (AUTO or MATLAB).
3. Connect the charged power bank and turn the power ON.
4. Optional: connect the Mac by USB to the left Nano and watch the messages:
   ```bash
   cd AutoFirmware
   pio device monitor
   ```
5. Right after the start the left Nano prints `IMU switch AN` or `IMU switch AUS`. From now on it remembers this position until the next power-off.

### AUTO mode

Nothing else to do. Every 30 s the Nano reads its clock, calculates the sun position and sends it to the right Nano. No computer needed.

The clock (DS1302) runs on **CET (winter time) all year**. Set it with `pio test -e nanosetup -f setup/test_rtc_set` (see [`test/README.md`](test/README.md)), only when the Mac is on winter time.

### MATLAB mode

1. Close the PlatformIO monitor (only one program can use the USB port).
2. Open `MatlabSerial/AusgabeSonnenstandSerial.m`, set `port` (find it with `serialportlist("available")`), click Run.
3. MATLAB sends the sun angle every 30 s. Stop with Ctrl+C, then type `clear s`.

**Never run the old `AusgabeSonnenstand.m` on this Nano.** Its `arduino()` call replaces AutoFirmware with MATLAB's own firmware.

You can also type angles by hand in the PlatformIO monitor: `azimuth elevation`, then Enter, e.g. `120 10`. Keep the elevation low while testing (the panel holder arm broke at 70° on 4 Nov 2026).

## What the tracker does

- **Night and unsafe angles:** if the sun is at or below the horizon, or its azimuth is outside 50°–310°, the panel parks: start direction, upright.
- **IMU AN:** every 2 s, while the mast is not moving, the left Nano reads the compass.
  - Every sent angle is corrected for the direction of the base.
  - If the base is turned by hand, the panel follows within a few seconds (when the needed angle changed by 6° or more).
  - If the compass fails 3 times in a row (about 6 s), the panel parks and the LED blinks fast. After 3 good readings in a row it goes back to the sun by itself.
  - If the needed angle is near 0°/360° (sun almost behind the base arrow), the mast waits at the edge and only crosses once the angle is 10° past it. This stops compass noise from spinning the mast back and forth.

## LED (left Nano)

| LED | Meaning |
|---|---|
| Slow blink | AUTO, everything OK |
| Always on | MATLAB, everything OK |
| Fast blink | Something is wrong. Read the message in the monitor (table below). |

## Messages and what to do

"Power-cycle" means: power OFF **and** unplug the USB cable (USB also powers the left Nano), wait a few seconds, then start again.

| Message | Meaning | What to do |
|---|---|---|
| `IMU switch AN` / `IMU switch AUS` | IMU switch read at the start | – |
| `AUTO    <time>  az=...  el=...  ack=0/0` | Sun angle sent; `ack=0/0` = the right Nano got both bytes | – |
| `... PARK` | Night or sun outside 50°–310° | – |
| `... arrow=...  phi=...` | IMU AN: base direction (`arrow`) and the corrected angle sent (`phi`) | – |
| `IMU     h=...  arrow=...` | Compass reading every 2 s (IMU AN) | – |
| `IMU     motor moving -> skip` | Mast is moving, compass not read (the motor disturbs it) | – |
| `RESEND ...` | Base was turned, corrected angle sent again | – |
| `ack=` with numbers other than `0/0` | Right Nano did not get the bytes | Check the I2C wires between the Nanos. |
| `AUTO    RTC stopped / not valid / frozen -> nothing sent` | Clock problem, nothing sent | Check the RTC wires (D5, D6, D7); set the clock again. |
| `ignored: switch on AUTO` | A MATLAB line arrived in AUTO | Flip the mode switch to MATLAB. |
| `MATLAB  bad line -> ignored` | Line was not `azimuth elevation` | Check the format (dot, not comma). |
| `MATLAB  timeout -> panel stays` | No good line for 180 s | Restart the MATLAB script. The panel keeps its position. |
| `MATLAB  ignored: IMU switch not known yet` | Line came in the first seconds after start | Wait a moment. |
| `right Nano not answering -> power-cycle` | Right Nano did not answer within 5 s of the start | Check the right Nano and the I2C wires, then power-cycle. |
| `IMU switch moved -> power-cycle` | IMU switch changed while running; panel parked, tracking stops | Do the alignment for the new position, then power-cycle. |
| `IMU     bad read (ignored)` | One bad compass reading | – (two are ignored) |
| `compass failed -> park` | 3 bad compass readings in a row | Check the IMU cable. |
| `... no heading -> ... PARK` | IMU AN but no good compass reading yet | Check the IMU cable; it goes back to tracking by itself. |
| `compass OK again` | 3 good readings after a failure | – |

## Stop

1. Turn the tracker's power OFF.
2. In MATLAB mode: Ctrl+C in MATLAB, then `clear s`.
3. Disconnect USB and the power bank if packing away.

The switches can stay where they are.