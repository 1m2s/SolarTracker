# Dashboard

Shows what the tracker is doing, live in the browser. Runs on the laptop only (127.0.0.1, no internet).

## Start

All commands run inside `Dashboard/`.

| Step | Command |
|---|---|
| Once: create the environment | `python3 -m venv .venv` |
| In every new terminal | `source .venv/bin/activate` |
| Once: install the packages | `pip install -r requirements.txt` |
| List the serial ports | `python bridge.py` |
| Terminal 1: start the bridge (keep it running) | `python bridge.py /dev/cu.wchusbserial130` |
| Terminal 2: start the dashboard | `python app.py` |

Then open `http://127.0.0.1:8080`. The port name changes with the USB socket.

## MATLAB through the bridge

With the bridge running, MATLAB sends the sun position through it, so MATLAB mode and the dashboard work at the same time.

1. Start the bridge first (Terminal 1 above).
2. Set the mode switch to **MATLAB**.
3. In MATLAB, run `MatlabSerial/AusgabeSonnenstandBridge.m`. There is no port to set.

| Where | What you see |
|---|---|
| MATLAB Command Window | `Connected to the bridge (127.0.0.1:5555)`, then every 30 s one `sent: ...` line with the azimuth and elevation, and the Nano's answer under it (`Nano: MATLAB  ...`). No `T:` lines: the script skips them. |
| Dashboard | Status shows `Mode: MATLAB`. `SUN` and `SEL` in the Values row are the angles MATLAB sent. Messages shows one `MATLAB  ...` line every 30 s. |
| MATLAB, when the bridge is not running | The script stops with `Could not connect to the bridge at 127.0.0.1:5555. Is bridge.py running?` |

The left Nano does not restart when the script connects, so the start sequence does not run again.

While the bridge runs it holds the USB port: `AusgabeSonnenstandSerial.m` and the PlatformIO monitor cannot open it. Stop the bridge first (Ctrl+C in Terminal 1) to use them.

Stop the script with Ctrl+C in MATLAB, then type `clear s`. The bridge and the dashboard keep running.
## The page

| Part | What it shows |
|---|---|
| Bridge | Whether the dashboard is connected to `bridge.py`. |
| Status | The place to look when something is wrong. One line with mode, IMU switch and start sequence, and the fault. Three more lines appear in red only when needed: what to do after a fault, the problem line while `ERR=1`, and the warning that the compass reads more than 5° away from power-on. Grey = no telemetry line for 3 s, the text is the last one known. |
| Panel | Panel voltage `U` in volts, current `I` in amps and power `P` in watts, calculated from `A0` and `A1`. A `?` means there is no reading (no finished sum yet, or after a fault). Grey = last known values. With nothing plugged into the USB socket, the small current and power shown are measurement noise. |
| Compass | Panel direction, sun, compass reading now and direction at power-on. |
| Values | The raw values of the telemetry line. Grey = old or never received. |
| Side view | The tracker seen from the side: the panel tilted by `EL` (0° = upright, 90° = flat) and the height of the sun `SEL`, both on a scale from 0° (the horizon) to 90° (straight up). Below the horizon the sun is an empty circle at 0°. The tilt is the last target the right Nano took, not a measured angle. The view compares heights only; the compass shows whether the panel is turned towards the sun. Grey = last known values. |
| Top view | The tracker seen from above with north up, on the same circle as the compass: the base plate as it stands on the ground (its board side faces the direction `PAN`), the panel as a bar with an arrow showing where it faces now, and the sun. The base is only drawn with the IMU switch AN, because `PAN` is only measured then. The panel direction comes from `AZ`, the last target the right Nano took, not a measured angle. Grey = last known values. |
| Graphs | Panel voltage, current and power over the last 10 minutes, one point per telemetry line. The axes are fixed (0 to 15 V, 0 to 1.5 A, 0 to 15 W), so the noise stays small. A break in a line means there was no reading. The graphs start empty every time `app.py` starts. |
| Messages | Every line from the Nano except the `T:` lines. The reason for a problem line is here. |

## Files

| File | What it does |
|---|---|
| `bridge.py` | Opens the USB port once and shares it: every line from the Nano goes to all connected programs, every line from a program goes to the Nano (TCP `127.0.0.1:5555`). |
| `link.py` | Reads the Nano's lines from the bridge in the background. Hands the `T:` lines to `telemetry.py`, adds their panel readings to `history.py` and keeps the latest messages. |
| `telemetry.py` | Turns a `T:` line into values and keeps the latest value of every key with the time it arrived. |
| `app.py` | The page in the browser. Only reads from `link.py`, `telemetry.py` and `history.py`, never from the serial port. |
| `angles.py` | Angle maths: the direction the panel faces now, and the difference between two directions. |
| `compass.py` | Draws the compass as an SVG picture. |
| `sideview.py` | Draws the side view (panel tilt and sun height) as an SVG picture. |
| `topview.py` | Draws the top view (base plate, panel and sun with north up) as an SVG picture. |
| `status.py` | Turns the codes `MODE`, `IMU`, `ST`, `FLT` and `ERR` into words for the Status block: the status line, what to do after a fault, the problem line and the moved-warning. |
| `power.py` | Turns the sums `A0` and `A1` into volts, amps and watts for the Panel block. |
| `history.py` | Keeps the last 10 minutes of volts, amps and watts for the graphs, one point per telemetry line. In memory only. |
| `test_*.py` | Tests for the file of the same name. |

## Tests

Inside `Dashboard/`, with the environment active:

```
pytest
```

The last line should say that all tests passed, for example `97 passed`.

## Telemetry line

The left Nano prints one line per second that starts with `T:`. All other lines are the normal messages from `AutoFirmware/HowToOperate.md`.

```
T:MODE=A ST=6 FLT=0 ERR=0 IMU=1 MOV=0 SUN=253.7 SEL=-10.4 PAN=303.2 HDG=303.5 AZ=56 EL=0 A0=2712 A1=19
```

Pairs of `key=value`, separated by one space. A key without a valid value is left out. The dashboard ignores keys it does not know.

| Key | Meaning | Values |
|---|---|---|
| `MODE` | Mode switch | `A` = AUTO, `M` = MATLAB |
| `ST` | Start sequence | 0 waiting, 1 settling, 2 measuring, 3 aiming, 4 turning, 5 holding, 6 done |
| `FLT` | Fault (stays until a power-cycle) | 0 none, 1 right Nano not answering, 2 IMU switch moved, 3 compass, 5 turn too long (4 is not used any more) |
| `ERR` | Problem that can go away again: clock problem, target not taken by the right Nano, or no MATLAB line for 180 s. The LED blinks fast; the Status block shows a problem line and the message log says which one. | 1 = yes, 0 = no |
| `IMU` | IMU switch | 1 = AN, 0 = AUS |
| `MOV` | Stepper moving | 1 = yes, 0 = no |
| `SUN` | Sun azimuth, last value calculated (AUTO) or received (MATLAB) | degrees from true north, 1 decimal |
| `SEL` | Sun elevation, same source | degrees, 1 decimal |
| `PAN` | Direction the panel faced at power-on, measured in the start sequence. Only with IMU AN. | degrees from true north, 1 decimal |
| `HDG` | The same direction measured now (live compass, display only). Left out while the stepper moves, during the start sequence and after a failed read. | degrees from true north, 1 decimal |
| `AZ` | Azimuth last sent to the right Nano. 180 = the direction at power-on. | degrees, steps of 2 |
| `EL` | Elevation last sent to the right Nano. 0 = panel upright. | degrees |
| `A0` | Panel voltage reading from the right Nano (pin A0, voltage divider): sum of 16 readings. Left out while the right Nano has no finished sum, did not answer, or after a fault. | 0 to 16368 |
| `A1` | Current reading from the right Nano (pin A1, across the 1/3 Ω measuring resistors): sum of 16 readings. Left out like `A0`. | 0 to 16368 |

### What the dashboard calculates

Direction the panel faces now, with IMU AN:

$$\text{panel}_{\text{now}} = \text{PAN} + (\text{AZ} - 180^\circ)$$

With IMU AUS the panel was placed facing South by hand, so $\text{panel}_{\text{now}} = \text{AZ}$.

If `HDG` and `PAN` differ by more than 5°, the tracker was moved after power-on (or something disturbs the compass). The Status block then shows a warning.

The left Nano asks the right Nano once per second. The right Nano finishes a new sum about every second (about every 2 s while the stepper turns), so a sum can show up twice. Sum ÷ 16 is the average reading, 0 to 1023.
Panel voltage, current and power (`power.py`), with $U_{\text{ref}} = 5.0\ \text{V}$ (the Nano's reference voltage, assumed, not measured yet):

$$U = \frac{\text{A0}}{16} \cdot \frac{U_{\text{ref}}}{1023} \cdot \frac{10\ \text{k}\Omega + 2.2\ \text{k}\Omega}{2.2\ \text{k}\Omega} \qquad I = \frac{\text{A1}}{16} \cdot \frac{U_{\text{ref}}}{1023} \cdot \frac{1}{\tfrac{1}{3}\ \Omega} \qquad P = U \cdot I$$

With nothing plugged into the USB socket no current flows. Pin A1 still reads a little then: a current up to about 0.05 A, and the power that goes with it, is measurement noise.
### Planned keys (right Nano, later)

| Key | Meaning |
|---|---|
| `PAZ` | Azimuth the stepper has really reached |
| `PEL` | Elevation the servo has really reached |

Voltage, current and power are calculated in the dashboard from `A0` and `A1`.