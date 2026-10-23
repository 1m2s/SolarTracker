# Debugging the current firmware

Run these commands in the MATLAB **Command Window**. Stop a running script with **Control+C** first.

Connect the Mac to the **left Arduino (`A_MATLAB`)**. For I2C checks, have the tracker powered normally through its charged power bank. Keep the mechanism clear because powering or resetting the controller may cause movement.

## 1. Clear the previous connection

```matlab
clear slave ar
```

- `ar` is the MATLAB object for the USB-connected Arduino.
- `slave` is the MATLAB object for the second controller on the I2C bus.
- `clear` removes these objects from MATLAB and releases the old connection. It **does not erase the firmware** on either Arduino.
- Clearing a variable that does not exist is harmless.

Use this before reconnecting, especially after the error **"MATLAB connection ... exists in your workspace."**

## 2. Connect to the first Arduino

```matlab
ar = arduino('/dev/cu.usbserial-AC013FMP','Nano3','Libraries','I2C')
```

| Part | Meaning |
| --- | --- |
| `arduino(...)` | Function supplied by the MATLAB Support Package for Arduino Hardware |
| `/dev/cu.usbserial-AC013FMP` | USB serial port observed on my Mac |
| `Nano3` | Arduino Nano V3 board type |
| `'Libraries','I2C'` | Enable I2C support for communication with the second controller |
| `ar =` | Store the connection under the programmer-chosen name `ar` |

**Success:** MATLAB displays the Arduino properties, including the port, board and I2C library. This establishes communication between the Mac and the first Arduino. It does not test the motors.

A semicolon at the end hides the normal output; it does not change the connection behavior. MATLAB may update its server firmware on the connected board when needed, so use the left Arduino, not the right motor controller.

## 3. Check the I2C bus

```matlab
scanI2CBus(ar)
```

This asks the first Arduino which I2C addresses respond. Look for **`0x19`**, the address expected for the second controller.

Example observed output:

```text
{'0x0C'}
{'0x19'}
{'0x68'}
{'0x77'}
```

Another successful scan contained only `0x19`, `0x68` and `0x77`. The extra addresses are not required to identify the tracker controller; their exact device identities have not been independently verified here.

**Success:** `0x19` appears. A device responds at the expected controller address. This does not establish that motor positioning or IMU readings are correct.

## 4. Set up communication with the second controller

```matlab
slave = device(ar,'I2CAddress','0x19','bitrate',100000);
```

- `ar`: use the first Arduino's connection.
- `0x19`: address of the second controller.
- `100000`: I2C communication speed of 100 kbit/s.
- `slave`: name used to store this device object; it is not a built-in command.

**Success:** MATLAB returns to `>>` without an error. The semicolon suppresses normal output.

This is the line that previously failed inside `AusgabeSonnenstand.m`. Testing it separately helps locate the failure before running the whole script.

## Quick connection check

Run the lines in order. If one fails, investigate that error before continuing.

```matlab
clear slave ar
ar = arduino('/dev/cu.usbserial-AC013FMP','Nano3','Libraries','I2C');
scanI2CBus(ar)
slave = device(ar,'I2CAddress','0x19','bitrate',100000);
```

These commands do not send target angles to the motors.

## If communication times out

For **"Unable to receive data from the target hardware"**:

1. Stop the MATLAB script if it is still running.
2. Check the charged power bank, its output and the existing power cable. Check whether the right Arduino has power.
3. Reconnect the Mac's USB cable to the left Arduino.
4. Run `clear slave ar`, create the connection again, then scan the I2C bus.
5. If the scan works, retry the `device(...)` line separately.

If restarting the tracker, follow its operating manual for the selected IMU mode and starting position. Do not move or rewire the mechanism while powered.

A successful USB connection followed by an I2C timeout narrows the problem to the I2C transaction or hardware state; it does not prove a faulty power bank. The intermittent timeout's root cause remains unconfirmed.

## If the Mac port changes

In **macOS Terminal**, not MATLAB, run:

```bash
ls /dev/cu.*
```

Compare the list with USB unplugged and plugged in. The new entry identifies the board's port. Update the port string both in the diagnostic command and in `AusgabeSonnenstand.m`.

## Normal operation

You do not need to run these diagnostics before every use. `AusgabeSonnenstand.m` already begins with `clear all` and creates its own Arduino and I2C device objects.

Once the tracker is correctly positioned and powered, run that script. It prints `a` (azimuth) and `g` (sun elevation) and sends targets every 30 seconds. Repeated values are normal because the calculation uses hours and minutes, not seconds.

To stop the script, click the Command Window and press **Control+C**. To stop motor power, use the tracker's physical **Power OFF** switch. Stopping MATLAB does not necessarily stop a movement already commanded.
