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

## Files

| File | What it does |
|---|---|
| `bridge.py` | Opens the USB port once and shares it: every line from the Nano goes to all connected programs, every line from a program goes to the Nano (TCP `127.0.0.1:5555`). |
| `link.py` | Reads the Nano's lines from the bridge in the background and keeps the latest ones. |
| `telemetry.py` | Turns a `T:` line into values and keeps the latest value of every key with the time it arrived. |
| `app.py` | The page in the browser. Only reads from `link.py` and `telemetry.py`, never from the serial port. |
| `test_telemetry.py` | Tests for `telemetry.py`. |

## Tests

Inside `Dashboard/`, with the environment active:

```
pytest
```

The last line should say `17 passed`.

## Telemetry line

The left Nano prints one line per second that starts with `T:`. All other lines are the normal messages from `AutoFirmware/HowToOperate.md`.

```
T:MODE=A ST=6 FLT=0 IMU=1 MOV=0 SUN=253.7 SEL=-10.4 PAN=303.2 HDG=303.5 AZ=56 EL=0
```

Pairs of `key=value`, separated by one space. A key without a valid value is left out. The dashboard ignores keys it does not know.

| Key | Meaning | Values |
|---|---|---|
| `MODE` | Mode switch | `A` = AUTO, `M` = MATLAB |
| `ST` | Start sequence | 0 waiting, 1 settling, 2 measuring, 3 aiming, 4 turning, 5 holding, 6 done |
| `FLT` | Fault | 0 none, 1 right Nano not answering, 2 IMU switch moved, 3 compass, 4 panel faces north, 5 turn too long |
| `IMU` | IMU switch | 1 = AN, 0 = AUS |
| `MOV` | Stepper moving | 1 = yes, 0 = no |
| `SUN` | Sun azimuth, last value calculated (AUTO) or received (MATLAB) | degrees from true north, 1 decimal |
| `SEL` | Sun elevation, same source | degrees, 1 decimal |
| `PAN` | Direction the panel faced at power-on, measured in the start sequence. Only with IMU AN. | degrees from true north, 1 decimal |
| `HDG` | The same direction measured now (live compass, display only). Left out while the stepper moves, during the start sequence and after a failed read. | degrees from true north, 1 decimal |
| `AZ` | Azimuth last sent to the right Nano. 180 = the direction at power-on. | degrees, steps of 2 |
| `EL` | Elevation last sent to the right Nano. 0 = panel upright. | degrees |

### What the dashboard calculates

Direction the panel faces now, with IMU AN:

$$\text{panel}_{\text{now}} = \text{PAN} + (\text{AZ} - 180^\circ)$$

With IMU AUS the panel was placed facing South by hand, so $\text{panel}_{\text{now}} = \text{AZ}$.

If `HDG` and `PAN` differ by more than a few degrees, the tracker was moved after power-on (or something disturbs the compass).

### Planned keys (right Nano, later)

| Key | Meaning |
|---|---|
| `A0` | Raw reading of the voltage divider (0–1023) |
| `A1` | Raw reading across the 1/3 Ω resistor (0–1023) |
| `PAZ` | Azimuth the stepper has really reached |
| `PEL` | Elevation the servo has really reached |

Voltage, current and power are calculated in the dashboard from `A0` and `A1`.