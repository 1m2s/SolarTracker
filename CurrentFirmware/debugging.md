# Debugging commands

## Find the USB port

Run in Mac Terminal:

```bash
ls /dev/cu.*
```

Lists serial ports. Unplug and reconnect the Arduino to identify which entry appears.

## Clear connections

```matlab
clear slave
clear ar
```

Removes `slave` and `ar` from MATLAB’s workspace. `clear` is a standard MATLAB command; `slave` and `ar` are names chosen by the programmer.

## Connect to the first Arduino

```matlab
ar = arduino('/dev/cu.usbserial-AC013FMP','Nano3','Libraries','I2C')
```

- `ar =`: stores the connection as `ar`.
- `arduino(...)`: function from MATLAB’s Arduino support package.
- `'/dev/cu.usbserial-AC013FMP'`: Arduino’s USB port on the Mac.
- `'Nano3'`: board type, Arduino Nano V3.
- `'Libraries','I2C'`: enables I²C communication.

## Check communication with the second Arduino

```matlab
scanI2CBus(ar)
```

Support-package function that scans the first Arduino’s I²C bus. Expect **`0x19`**, the second Arduino’s address.

## Set up the second Arduino connection

```matlab
slave = device(ar,'I2CAddress','0x19','bitrate',100000);
```

- `slave =`: stores the device object as `slave`.
- `device(...)`: function from MATLAB’s Arduino support package.
- `ar`: communicates through the first Arduino.
- `'I2CAddress','0x19'`: selects the second Arduino’s address.
- `'bitrate',100000`: sets I²C speed to 100,000 bits/s.
- `;`: hides normal output.