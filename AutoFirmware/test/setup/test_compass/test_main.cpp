/* Notes
   Setup tool: reads the compass and prints the heading for 20 s.
   It WRITES settings into the IMU (wake up, bypass, start a
   measurement). Nothing moves, and the settings are gone at power-off.
   Uses the Imu module (src/Imu.cpp), the same code main.cpp will use.
   While it runs: turn the platform slowly by hand and compare
   the heading with a phone compass.
   Needs: tracker on, USB to left Nano, MotorFirmware on the right Nano,
          stepper not moving.
   Run: pio test -e nanosetup -f setup/test_compass -v
*/

#include <Arduino.h>
#include <Wire.h>
#include <unity.h>
#include <stdio.h>
#include "Imu.h"
#include "Heading.h"

const uint8_t  SAMPLES   = 40;     // 40 x 0.5 s = 20 s
const uint16_t PAUSE_MS  = 500;

void setUp(void) {}
void tearDown(void) {}

// 1) Same start as the old firmware: wake up, +-2 g, bypass on (Imu.cpp).
void test_wake_up_imu(void)
{
    uint8_t id = 0;
    TEST_ASSERT_TRUE_MESSAGE(imuWhoAmI(id), "MPU-9250 does not answer");
    TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x71, id, "not an MPU-9250");
    TEST_ASSERT_TRUE_MESSAGE(imuBegin(), "wake-up failed or compass (0x0C) does not answer");
}

// 2) Print the heading every 0.5 s. Turn the platform while it runs.
//    A bad read prints "bad read" and the test goes on (unplug test).
void test_print_heading(void)
{
    char    line[64];
    uint8_t badReads = 0;

    for (uint8_t i = 0; i < SAMPLES; i++) {
        ImuRaw raw = {0, 0, 0, 0, 0};
        if (imuRead(raw)) {
            float Mx, My, Mz;
            magCalibrated(raw.magX, raw.magY, raw.magZ, Mx, My, Mz);
            float h = tiltHeading(raw.accelX / 16384.0f, raw.accelY / 16384.0f, Mx, My, Mz);

            // AVR snprintf has no %f -> whole numbers only
            snprintf(line, sizeof(line), "h=%3d  raw m=%5d %5d %5d  a=%6d %6d",
                     (int)(h + 0.5f), raw.magX, raw.magY, raw.magZ, raw.accelX, raw.accelY);
        } else {
            badReads++;
            snprintf(line, sizeof(line), "bad read");
        }
        TEST_MESSAGE(line);
        delay(PAUSE_MS);
    }

    snprintf(line, sizeof(line), "bad reads: %u of %u", (unsigned)badReads, (unsigned)SAMPLES);
    TEST_MESSAGE(line);
    TEST_ASSERT_TRUE_MESSAGE(badReads < SAMPLES, "every read was bad");
}

void setup()
{
    delay(2000);                       // wait for the Serial monitor
    Wire.begin();                      // this Nano is the I2C master
    Wire.setWireTimeout(3000, true);   // never hang on a stuck bus
    UNITY_BEGIN();
    RUN_TEST(test_wake_up_imu);
    RUN_TEST(test_print_heading);
    UNITY_END();
}

void loop() {}

/* Notes
1) Same register steps as the old firmware (CurrentFirmware), now in
   Imu.cpp: initialize() woke the MPU (0x6B = 0x01) with +-2 g
   (0x1C = 0x00), getMotion9() turned bypass on (0x37 = 0x02), then each
   compass reading was: 0x0A = 0x01 (measure once), wait 10 ms, read.
   Imu.cpp reads 7 bytes (the 7th has the overflow flag) and averages
   4 readings, so h is steadier than on 4 Nov (single readings).

2) Bypass connects the compass chip inside the MPU-9250 to our bus.
   Without it, 0x0C is invisible (that is why the scan never saw it).

3) Byte order differs: the accelerometer sends the high byte first,
   the compass sends the low byte first.

4) The heading math is Heading.cpp, tested in logic/test_heading.
   This tool only checks the real sensor.

5) What to look for:
   - turn the platform 90 deg -> h changes by about 90
   - turn it back -> h comes back
   - compare with a phone compass (only roughly, the phone is not exact)
   - unplug the IMU for about 3 s -> "bad read" lines; plug it back ->
     normal lines again (imuRead wakes the IMU up again after a bad read)
   Write down the results, they decide how good the compass mode can be.
*/