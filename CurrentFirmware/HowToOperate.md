# How to operate

Both modes use MATLAB to calculate the sun’s position. The IMU switch selects how the tracker determines its starting direction.

## IMU OFF — manual alignment

1. Keep Power OFF.
2. Set the IMU switch OFF.
3. Carefully position the panel upright using the servo linkage.
4. Turn the whole base so the solar-cell face points south.
5. Keep the base level.

## IMU ON — compass alignment

1. Keep Power OFF.
2. Set the IMU switch ON.
3. Carefully position the panel upright using the servo linkage.
4. Align the panel relative to the base as shown in Figure 2, page 2 of `Bedienungsanleitung.pdf`. ![Panel starting position for IMU mode](/images/figure2.png)
5. Keep the base level. The base does not need to face south.

The IMU measures compass direction. Keep it away from nearby magnets and metal objects.

## Start

1. Complete the alignment for the selected mode.
2. Connect the charged power bank through the existing power-input cable and activate its output.
3. If switching modes, hold RESET on the right Arduino.
4. Turn Power ON, then release RESET if held.
5. Connect the Mac by USB to the left Arduino (`A_MATLAB`).
6. Open `AusgabeSonnenstand.m` in MATLAB.
7. Check the USB port in the `arduino(...)` line.
8. Click Run.

MATLAB prints:
- `a`: sun azimuth in degrees.
- `g`: sun elevation in degrees.

Targets are sent every 30 seconds.

## Stop

1. Click MATLAB’s Command Window and press Control+C.
2. Turn the tracker’s Power OFF.
3. Disconnect USB and the power bank if packing away.

The IMU switch can stay in its selected position.