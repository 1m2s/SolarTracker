# Tests (MotorFirmware)

All commands run inside `MotorFirmware/`.

| Folder | What it checks | Needs | Runs in CI |
|---|---|---|---|
| `logic/` | Step and servo rules (`test_motion`), voltage and power (`test_measure`), status byte (`test_status`) | Nothing | ✅ |

| Goal | Command |
|---|---|
| Logic on the PC (same as CI) | `pio test -e native` |
| Build the firmware | `pio run` |
| ⚠️ Flash the right Nano (USB on the RIGHT Nano) | `pio run -t upload` |