# Tests

All commands run inside `AutoFirmware/`.

## Folders

| Folder | What it checks | Needs | Runs in CI |
|---|---|---|---|
| `logic/` | Sun-position math and byte conversion vs. MATLAB, switch debounce, AUTO timing, MATLAB timeout and LED pattern (`test_controller`), 30 s timer across the millis() wrap (`test_isDue`), MATLAB line parser (`test_parseAngles`), collecting USB characters into lines (`test_lineReader`), compass heading (`test_heading`), compass correction, edge rule and 6° re-send rule (`test_correction`) | Nothing | ✅ |
| `hardware/` | Real tracker, **read-only** | Tracker on, USB to left Nano | ❌ |
| `setup/` | Tools that **write** to hardware | Tracker on, USB to left Nano | ❌ |

⚠️ With CurrentFirmware on the right Nano, **IMU switch AN breaks the I2C bus**: no device answers (see `test_i2c_fullscan`, result 2 Nov 2026). With `MotorFirmware` on the right Nano this no longer happens, because the right Nano never starts IMU mode. The switch is only read and passed on (`test_imu_switch`).

## Before every commit

Run these in `AutoFirmware/`. All must pass before `git commit`.

| # | Check | Command |
|---|---|---|
| 1 | Logic tests on the PC (same as CI) | `pio test -e native` |
| 2 | No warnings in our code (strict flags, all of `src/` except `main.cpp` and `Imu.cpp`) | `bash tools/check_warnings.sh` |
| 3 | Nano build without warnings (includes `main.cpp` and `Imu.cpp`) | `PLATFORMIO_BUILD_SRC_FLAGS="-Wall -Wextra -Wshadow -Werror" pio run -e nanoatmega328` |
| 4 | Only if a file used by a `setup/` or `hardware/` test changed: build that test without flashing | `pio test -e nanosetup -f setup/<name> --without-uploading --without-testing` (`-e nanotest` for `hardware/`) |
| 5 | Only if `MotorFirmware/` changed (run in `MotorFirmware/`) | `pio test -e native` and `pio run` |
| 6 | Only if `main.cpp` or anything on the tracker changed: flash and test on the tracker | `pio run -e nanoatmega328 -t upload && pio device monitor` |

After `git push`: check the green tick on GitHub (Actions). CI repeats 1–3, runs cppcheck on `src/` and builds and tests `MotorFirmware/`.

cppcheck on the Mac (optional, same as CI):

```bash
pip install cppcheck==1.5.3
cppcheck --enable=warning,performance,portability --std=c++11 --language=c++ -Iinclude --suppress=missingIncludeSystem --inline-suppr --error-exitcode=1 --quiet src/
```

## Commands

| Goal | Command |
|---|---|
| Math on the PC (same as CI) | `pio test -e native` |
| Math on the Nano | `pio test -e nanotest -f "logic/*"` |
| All hardware checks | `pio test -e nanotest -f "hardware/*"` |
| Only the I2C scan | `pio test -e nanotest -f hardware/test_i2c_scan` |
| Full scan, 5 knocks per address (results only with `-v`) | `pio test -e nanotest -f hardware/test_i2c_fullscan -v` |
| Status byte of the right Nano: IMU switch + stepper moving (results only with `-v`) | `pio test -e nanotest -f hardware/test_imu_switch -v` |
| IMU alive? (`WHO_AM_I`, results only with `-v`) | `pio test -e nanotest -f hardware/test_imu_id -v` |
| Only the clock | `pio test -e nanotest -f hardware/test_rtc` |
| Live clock vs. Mac (after `test_rtc`) | `~/.platformio/penv/bin/python test/hardware/test_rtc/check_rtc.py` |
| Watch the clock (Mac time on the left) | `pio device monitor -b 115200 --filter time` |
| ⚠️ Set the clock (winter time only!) | `pio test -e nanosetup -f setup/test_rtc_set` |
| ⚠️ I2C link check (sends start values, nothing moves; mode "ohne IMU", fresh power-on) | `pio test -e nanosetup -f setup/test_i2c_link` |
| ⚠️ Status bit 1 check (MOVES the mast 180° → 200° → 180°; MotorFirmware, fresh power-on, panel south) | `pio test -e nanosetup -f setup/test_status_moving -v` |
| ⚠️ Compass live for 20 s (writes temporary IMU settings, nothing moves; turn the platform by hand) | `pio test -e nanosetup -f setup/test_compass -v` |
| ⚠️ Compass calibration run: exact 180° turn, table edge to table edge (do it 3 times: run1, run2, run3) | `pio test -e nanosetup -f setup/test_compass_cal -v \| tee test/setup/test_compass_cal/run1.txt` |
| Compute the calibration from the 3 runs (prints the numbers for `include/config/Calibration.h`) | `~/.platformio/penv/bin/python test/setup/test_compass_cal/fit_compass.py test/setup/test_compass_cal/run1.txt test/setup/test_compass_cal/run2.txt test/setup/test_compass_cal/run3.txt` |
| Make MATLAB reference values | MATLAB: `cd test/logic/test_sunPosition`, then `generate_reference` |