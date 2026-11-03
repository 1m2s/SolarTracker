# MATLAB to C++ mapping

The sun-position math in `AutoFirmware` is a line-by-line port of the MATLAB code in `../CurrentFirmware/Matlab/`.
Names were translated to English and given a unit suffix (`Deg`, `Rad`, `Min`); the formulas and their order were kept.

## 1. Functions

| MATLAB file | MATLAB function | C++ function | C++ file |
|---|---|---|---|
| `grad2rad.m` | `grad2rad` | `degToRad` | `AngleConversion.cpp` |
| `rad2grad.m` | `rad2grad` | `radToDeg` | `AngleConversion.cpp` |
| `JahresTag.m` | `JahresTag` | `dayOfYear` | `SolarTime.cpp` |
| `MEZ.m` | `MEZ` | `cetMinutes` | `SolarTime.cpp` |
| `MOZ.m` | `MOZ` | `meanLocalMinutes` | `SolarTime.cpp` |
| `Zeitgleichung.m` | `Zeitgleichung` | `equationOfTime` | `SolarTime.cpp` |
| `StundenWinkel.m` | `StundenWinkel` | `hourAngle` | `SolarTime.cpp` |
| `Deklination.m` | `Deklination` | `declination` | `SunAngles.cpp` |
| `HoehenWinkel.m` | `HoehenWinkel` | `elevation` | `SunAngles.cpp` |
| `Azimut.m` | `Azimut` | `azimuth` | `SunAngles.cpp` |
| `SonnenStand.m` | `SonnenStand` | `sunPosition` | `SunPosition.cpp` |
| `AusgabeSonnenstand.m` | (script) | `autoCycle`, `sendTarget`, `writeRegister` | `main.cpp` |
| | | `toUint8`, `targetBytes` | `MotorTarget.cpp` |
| | | `AUTO_INTERVAL_MS`, `LATITUDE_DEG`, `LONGITUDE_DEG` | `config/Settings.h` |
| | | `SLAVE_ADDR`, `REG_AZIMUTH`, `REG_ELEVATION` | `config/Board.h` |

## 2. Variables

### grad2rad.m → degToRad

| MATLAB | C++ | Unit |
|---|---|---|
| `grad` | `angleDeg` | ° |
| `rad` | return value | rad |

### rad2grad.m → radToDeg

| MATLAB | C++ | Unit |
|---|---|---|
| `rad` | `angleRad` | rad |
| `grad` | return value | ° |

### JahresTag.m → dayOfYear

| MATLAB | C++ | Unit |
|---|---|---|
| `Monat` | `month` | 1..12 |
| `Tag` | `day` | 1..31 |
| `MTage` | `daysInMonth` | days |
| `i` (1-based) | `monthIndex` (0-based) | index |
| `d` | `dayNumber` | day of year, 1..365 |

### MEZ.m → cetMinutes

| MATLAB | C++ | Unit |
|---|---|---|
| `Stunde` | `hour` | h |
| `Minute` | `minute` | min |
| `tf` | `summerTime` | bool (MATLAB: 0/1) |
| `MEZ_Stunde` | `cetHour` | h |
| `MEZ_Minute` | `cetMinute` | min |
| `Min_MEZ` | `cetMin` | min since 0:00 CET |

### MOZ.m → meanLocalMinutes

| MATLAB | C++ | Unit |
|---|---|---|
| `Min_MEZ` | `cetMin` | min since 0:00 CET |
| `lambda` | `longitudeDeg` | ° east |
| `Min_MOZ` | `meanLocalMin` | min, mean local time |

### Zeitgleichung.m → equationOfTime

| MATLAB | C++ | Unit |
|---|---|---|
| `Jg` | `yearAngleDeg` | ° |
| `a1`, `a2`, `a3` | `term1Rad`, `term2Rad`, `term3Rad` | rad |
| `zgl` | `equationOfTimeMin` | min |

### StundenWinkel.m → hourAngle

| MATLAB | C++ | Unit |
|---|---|---|
| `Min_WOZ` | `trueSolarMin` | min, true solar time |
| `WOZ_h` | `trueSolarHours` | h |
| `beta` | `hourAngleDeg` | ° |

### Deklination.m → declination

| MATLAB | C++ | Unit |
|---|---|---|
| `d` | `dayNumber` | day of year |
| `Js` | `yearAngleDeg` | ° (the MATLAB comment says rad, but it is °) |
| `a1`, `a2`, `a3` | `term1Rad`, `term2Rad`, `term3Rad` | rad |
| `delta` | `declinationDeg` | ° |

### HoehenWinkel.m → elevation

| MATLAB | C++ | Unit |
|---|---|---|
| `beta` | `hourAngleDeg` | ° |
| `phi` | `latitudeDeg` | ° north |
| `delta` | `declinationDeg` | ° |
| `beta_r` | `hourAngleRad` | rad |
| `phi_r` | `latitudeRad` | rad |
| `delta_r` | `declinationRad` | rad |
| `gamma` (after `asin`) | `elevationRad` | rad |
| `gamma` (after `rad2grad`) | `elevationDeg` | ° |

### Azimut.m → azimuth

| MATLAB | C++ | Unit |
|---|---|---|
| `d` | `dayNumber` | day of year |
| `phi` | `latitudeDeg` | ° north |
| `WOZ` | `trueSolarMin` | min, true solar time |
| `rphi` | `latitudeRad` | rad |
| `beta` | `hourAngleDeg` | ° |
| `rbeta` | `hourAngleRad` | rad |
| `delta` | `declinationDeg` | ° |
| `rdelta` | `declinationRad` | rad |
| `rgamma` | `elevationRad` | rad |
| `gamma` | (left out, unused) | ° |
| `i` | (no loop) | index |
| `cos(rgamma(i)).*cos(rphi)` | `divisor` | – |
| `acos` argument | `cosArgument` | – |
| `alpha` (before `rad2grad`) | `azimuthRad` | rad |
| `alpha` (after `rad2grad`) | `azimuthDeg` | °, 180 = south |

### SonnenStand.m → sunPosition

| MATLAB | C++ | Unit |
|---|---|---|
| `Breite` | `latitudeDeg` | ° north |
| `Laenge` | `longitudeDeg` | ° east |
| `Tag` | `day` | 1..31 |
| `Monat` | `month` | 1..12 |
| `Stunde` | `hour` | h |
| `Minute` | `minute` | min |
| `tf` | `summerTime` | bool |
| `Min_MEZ` | `cetMin` | min |
| `Min_MOZ` | `meanLocalMin` | min |
| `d` | `dayNumber` | day of year |
| `Jg` | `yearAngleDeg` | ° |
| `zgl` | `equationOfTimeMin` | min |
| `Min_WOZ` | `trueSolarMin` | min |
| `delta` | `declinationDeg` | ° |
| `beta` | `hourAngleDeg` | ° |
| `alpha` (output) | `azimuthDeg` (reference) | ° |
| `gamma` (output) | `elevationDeg` (reference) | ° |

### AusgabeSonnenstand.m → main.cpp, MotorTarget, config

| MATLAB | C++ | Unit / note |
|---|---|---|
| `Laenge = 9.66` | `LONGITUDE_DEG` (`Settings.h`) | ° east |
| `Breite = 47.81` | `LATITUDE_DEG` (`Settings.h`) | ° north |
| `slave = device(ar,'I2CAddress','0x19',...)` | `SLAVE_ADDR` (`Board.h`), `Wire.begin()` | 0x19 |
| `'bitrate',100000` | Wire default | 100 kHz |
| `while 1` ... `pause(30)` | `loop()` + `ctrlAutoDue` + `AUTO_INTERVAL_MS` | 30000 ms |
| `[c,tf] = clock` | `rtcTime = Rtc.GetDateTime()`, `RTC_SUMMER_TIME` | DS1302 RTC |
| `Jahr`, `Monat`, `Tag`, `Stunde`, `Minute` | `rtcTime.Year()`, `.Month()`, `.Day()`, `.Hour()`, `.Minute()` | |
| `SonnenStand(...)` | `sunPosition(...)` in `autoCycle` | |
| `a`, `g` | `azimuthDeg`, `elevationDeg` | ° |
| `a = a / 2` | `azimuthDeg / 2.0f` in `targetBytes` | ° / 2 |
| `uint8(a)`, `uint8(g)` | `toUint8(...)` → `azimuthByte`, `elevationByte` | 0..255 |
| `writeRegister(slave,254,a,'uint8')` | `writeRegister(REG_AZIMUTH, azimuthByte)` in `sendTarget` | register 254 |
| `writeRegister(slave,255,g,'uint8')` | `writeRegister(REG_ELEVATION, elevationByte)` in `sendTarget` | register 255 |

`toUint8` behaves like MATLAB `uint8`: rounds half away from zero, saturates at 0 and 255, NaN → 0.

### MatlabSerial/AusgabeSonnenstandSerial.m → MATLAB mode

This script is new (not a port). It is the MATLAB side of the C++ MATLAB mode and calls the original `SonnenStand.m` unchanged.

| MATLAB | C++ | Unit / note |
|---|---|---|
| `serialport(port, 115200)` | `SERIAL_BAUD` (`Settings.h`) | baud |
| `writeline(s, msg)` | `readSerial` → `lineFeed` → `handleLine` (`main.cpp`, `MatlabLink.cpp`) | line ends with `\n` |
| `sprintf("%.4f %.4f", a, g)` | `parseAngles(line, azimuthDeg, elevationDeg)` | ° |
| `real(a)` | clamp of `cosArgument` in `azimuth` | same result |
| Nano parks at night | `targetBytes` via `sendTarget` | |
| 180 s timeout (notes) | `MATLAB_TIMEOUT_MS`, `ctrlMatlabTimedOut` | 180000 ms |
| "ignored: switch on AUTO" | `handleLine` | |

## 3. Deliberate differences from MATLAB

| Where | Difference |
|---|---|
| `cetMinutes` | `tf` (number 0/1) became `bool summerTime`; MATLAB leaves `MEZ_Stunde` undefined for any other value. |
| `azimuth` | Computes one value instead of looping over a vector; the unused `gamma = rad2grad(rgamma)` line is left out. |
| `azimuth` | The `acos` argument is clamped to -1..1 (MATLAB gives a complex number there). |
| `azimuth` | Returns `NAN` when `divisor` is below `AZIMUTH_DIVISOR_MIN` (1e-3, sun straight overhead). |
| `azimuth` | Computes the elevation formula itself, like `Azimut.m` (no call to `elevation()`). |
| all math | `float` and `sinf`/`cosf`/... instead of `double` (on the AVR, `double` is also 32 bit). |
| `dayOfYear` | No leap years, same as MATLAB (not a difference, kept on purpose). |
| `targetBytes` | Parks (azimuth byte 90, elevation byte 0) at night (`elevationDeg <= 0`) or when the azimuth is outside 50..310 ° or `NAN`. MATLAB sends the raw bytes. |
| time source | MATLAB `clock` gives PC time and the summer-time flag; C++ reads the DS1302 RTC and uses the fixed `RTC_SUMMER_TIME = false` (RTC kept on winter time). |
| `Jahr` | Unused in MATLAB; C++ uses the year only to check the RTC (`RTC_YEAR_MIN`). |
| interval | MATLAB waits 30 s after each send; C++ runs every 30 s from the start of the last cycle (`isDue`), and runs right away on start-up or on switching to AUTO (`runNow`). |
| I2C errors | C++ checks the ack of each `writeRegister` and shows an error on the LED (`ctrlAutoResult`); `SEND_I2C` can turn sending off. |
| C++ only | RTC checks in `autoCycle` (stopped, not valid, `clockFrozen`), `TimeChecks` (`isDue`, `clockFrozen`), MATLAB mode over USB (`MatlabLink`), `Controller` (AUTO/MATLAB switch, timeout, LED), `Heading` (compass; used by tests only, not by `main.cpp`). |
