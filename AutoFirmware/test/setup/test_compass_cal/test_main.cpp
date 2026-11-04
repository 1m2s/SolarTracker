/* Notes
   Setup tool: records raw compass values for the calibration.
   It WRITES temporary settings into the IMU (wake up, bypass, measure).
   Nothing moves, nothing is stored on the Nano.
   What to do: put one straight side of the base against a table edge.
   Hold still. When "TURN NOW" appears, turn the whole tracker slowly
   EXACTLY 180 deg, until the same side lies against the same table edge
   again. Be there before 25 s and hold still until "STOP".
   The values are printed as "CAL x y z" lines. fit_compass.py (same
   folder) computes the calibration for Calibration.h from 3 such runs.
   Needs: tracker on, USB to left Nano, MotorFirmware on the right Nano.
   Run (from AutoFirmware/), three times with run1, run2, run3:
     pio test -e nanosetup -f setup/test_compass_cal -v | tee test/setup/test_compass_cal/run1.txt
   Then:
     ~/.platformio/penv/bin/python test/setup/test_compass_cal/fit_compass.py test/setup/test_compass_cal/run1.txt test/setup/test_compass_cal/run2.txt test/setup/test_compass_cal/run3.txt
*/

#include <Arduino.h>
#include <Wire.h>
#include <unity.h>
#include <stdio.h>
#include "Imu.h"

// ---------- settings ----------
const uint32_t TURN_MS   = 30000;   // 30 s for half a circle
const uint32_t SAMPLE_MS = 200;     // one CAL line every 0.2 s

uint16_t samples = 0;

void setUp(void) {}
void tearDown(void) {}

// 1) Wake up the IMU and turn bypass on, then check the compass (Imu.cpp).
void test_wake_up(void)
{
    TEST_ASSERT_TRUE_MESSAGE(imuBegin(), "MPU-9250 or compass (0x0C) does not answer");
}

// 2) 30 s of raw values while you turn exactly 180 deg.
void test_record_half_circle(void)
{
    char line[40];

    TEST_MESSAGE("Hold still on the start mark. Turning starts in 3 s");
    delay(3000);
    TEST_MESSAGE("TURN NOW: exactly 180 deg in about 25 s, then hold still");

    uint32_t start    = millis();
    uint32_t lastLine = start;

    while (millis() - start < TURN_MS) {
        if (millis() - lastLine < SAMPLE_MS) {
            continue;
        }
        lastLine += SAMPLE_MS;

        ImuRaw raw = {0, 0, 0, 0, 0};
        TEST_ASSERT_TRUE(imuRead(raw));   // average of 4 readings
        snprintf(line, sizeof(line), "CAL %d %d %d", raw.magX, raw.magY, raw.magZ);
        TEST_MESSAGE(line);
        samples++;
    }
    TEST_MESSAGE("STOP turning. Now run fit_compass.py on the saved output.");
    TEST_ASSERT_TRUE_MESSAGE(samples > 50, "too few samples");
}

void setup()
{
    delay(2000);                       // wait for the Serial monitor
    Wire.begin();                      // this Nano is the I2C master
    Wire.setWireTimeout(3000, true);   // never hang on a stuck bus
    UNITY_BEGIN();
    RUN_TEST(test_wake_up);
    RUN_TEST(test_record_half_circle);
    UNITY_END();
}

void loop() {}

/* Notes
1) Why exactly 180 deg: in a 180 deg turn the start and the end point lie
   exactly opposite each other around the true centre, even when iron
   near the sensor squashes the circle into an oval. So fit_compass.py
   can find the centre as the point halfway between them, and then the
   oval around it. The first and the last 2 s must be still for that.

2) Why not a full circle: the cables only allow half a turn.
   CurrentFirmware used (max + min) / 2 over a full circle instead.

3) The Nano only records. The maths is in fit_compass.py on the Mac,
   so the raw data stays in run1.txt .. run3.txt and can be checked again.

4) Each line is the average of 4 readings (MAG_SAMPLES in Settings.h,
   done by imuRead in Imu.cpp), noise about +-3. Same as before the move.
*/