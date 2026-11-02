# Tests

All commands run inside `AutoFirmware/`.

## Folders

| Folder | What it checks | Needs | Runs in CI |
|---|---|---|---|
| `logic/` | Sun-position math and byte conversion vs. MATLAB, LED pattern (`test_ledPattern`), 30 s timer across the millis() wrap (`test_isDue`), MATLAB line parser (`test_parseAngles`), collecting USB characters into lines (`test_lineReader`) | Nothing | ✅ |
| `hardware/` | Real tracker, **read-only** | Tracker on, IMU switch AUS, USB to left Nano | ❌ |
| `setup/` | Tools that **write** to hardware | Tracker on, IMU switch AUS, USB to left Nano | ❌ |

⚠️ With the thesis firmware on the right Nano, **IMU switch AN breaks the I2C bus**: no device answers (see `test_i2c_fullscan`, result 2 Nov 2026). Keep the switch on AUS for all hardware tests.

## Commands

| Goal | Command |
|---|---|
| Math on the PC (same as CI) | `pio test -e native` |
| Math on the Nano | `pio test -e nanotest -f "logic/*"` |
| All hardware checks | `pio test -e nanotest -f "hardware/*"` |
| Only the I2C scan | `pio test -e nanotest -f hardware/test_i2c_scan` |
| Full scan, 5 knocks per address (results only with `-v`) | `pio test -e nanotest -f hardware/test_i2c_fullscan -v` |
| Only the clock | `pio test -e nanotest -f hardware/test_rtc` |
| Live clock vs. Mac (after `test_rtc`) | `~/.platformio/penv/bin/python test/hardware/test_rtc/check_rtc.py` |
| Watch the clock (Mac time on the left) | `pio device monitor -b 115200 --filter time` |
| ⚠️ Set the clock (winter time only!) | `pio test -e nanosetup -f setup/test_rtc_set` |
| ⚠️ I2C link check (sends start values, nothing moves; mode "ohne IMU", fresh power-on) | `pio test -e nanosetup -f setup/test_i2c_link` |
| Make MATLAB reference values | MATLAB: `cd test/logic/test_sonnenstand`, then `generate_reference` |