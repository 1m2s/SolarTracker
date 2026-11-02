/* Notes
   Setup tool: reads the compass and prints the heading for 20 s.
   It WRITES settings into the IMU (wake up, bypass, start a
   measurement). Nothing moves, and the settings are gone at power-off.
   While it runs: turn the platform slowly by hand and compare
   the heading with a phone compass.
   Needs: tracker on, USB to left Nano, MotorFirmware on the right Nano,
          stepper not moving.
   Run: pio test -e nanosetup -f setup/test_compass -v
*/

#include <Arduino.h>
#include <Wire.h>
#include <unity.h>
#include "Heading.h"

// ---------- MPU-9250 (accelerometer) ----------
const uint8_t MPU_ADDR         = 0x68;
const uint8_t REG_WHO_AM_I     = 0x75;
const uint8_t REG_PWR_MGMT_1   = 0x6B;
const uint8_t REG_ACCEL_CONFIG = 0x1C;
const uint8_t REG_INT_PIN_CFG  = 0x37;
const uint8_t REG_ACCEL_XOUT_H = 0x3B;

// ---------- AK8963 (compass, inside the MPU-9250) ----------
const uint8_t MAG_ADDR     = 0x0C;
const uint8_t REG_WIA      = 0x00;
const uint8_t REG_HXL      = 0x03;
const uint8_t REG_CNTL1    = 0x0A;

const uint8_t  SAMPLES   = 40;     // 40 x 0.5 s = 20 s
const uint16_t PAUSE_MS  = 500;

bool    allZero = true;            // did the compass ever give a value?

// ---------- small I2C helpers ----------
bool writeReg(uint8_t dev, uint8_t reg, uint8_t value)
{
    Wire.beginTransmission(dev);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

bool readRegs(uint8_t dev, uint8_t reg, uint8_t count, uint8_t *buf)
{
    Wire.beginTransmission(dev);
    Wire.write(reg);
    if (Wire.endTransmission() != 0) {
        return false;
    }
    if (Wire.requestFrom(dev, count) != count) {
        return false;
    }
    for (uint8_t i = 0; i < count; i++) {
        buf[i] = Wire.read();
    }
    return true;
}

void setUp(void) {}
void tearDown(void) {}

// 1) Same start as the old firmware: wake up, +-2 g, bypass on.
void test_wake_up_imu(void)
{
    uint8_t id = 0;
    TEST_ASSERT_TRUE_MESSAGE(readRegs(MPU_ADDR, REG_WHO_AM_I, 1, &id), "MPU-9250 does not answer");
    TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x71, id, "not an MPU-9250");

    TEST_ASSERT_TRUE(writeReg(MPU_ADDR, REG_PWR_MGMT_1, 0x01));     // clock from gyro, no sleep
    TEST_ASSERT_TRUE(writeReg(MPU_ADDR, REG_ACCEL_CONFIG, 0x00));   // +-2 g -> 16384 per g
    TEST_ASSERT_TRUE(writeReg(MPU_ADDR, REG_INT_PIN_CFG, 0x02));    // bypass on
    delay(10);
}

// 2) After bypass the compass chip must show up at 0x0C.
void test_compass_answers(void)
{
    uint8_t wia = 0;
    TEST_ASSERT_TRUE_MESSAGE(readRegs(MAG_ADDR, REG_WIA, 1, &wia), "compass (0x0C) does not answer");
    TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x48, wia, "0x0C is not an AK8963");
}

// 3) Print the heading every 0.5 s. Turn the platform while it runs.
void test_print_heading(void)
{
    uint8_t acc[6];
    uint8_t mag[6];
    char line[64];

    for (uint8_t i = 0; i < SAMPLES; i++) {
        // Accelerometer: high byte first
        TEST_ASSERT_TRUE(readRegs(MPU_ADDR, REG_ACCEL_XOUT_H, 6, acc));
        int16_t ax = (int16_t)((acc[0] << 8) | acc[1]);
        int16_t ay = (int16_t)((acc[2] << 8) | acc[3]);

        // Compass: one measurement, wait, read. Low byte first.
        TEST_ASSERT_TRUE(writeReg(MAG_ADDR, REG_CNTL1, 0x01));
        delay(10);
        TEST_ASSERT_TRUE(readRegs(MAG_ADDR, REG_HXL, 6, mag));
        int16_t mx = (int16_t)((mag[1] << 8) | mag[0]);
        int16_t my = (int16_t)((mag[3] << 8) | mag[2]);
        int16_t mz = (int16_t)((mag[5] << 8) | mag[4]);
        if (mx != 0 || my != 0 || mz != 0) {
            allZero = false;
        }

        float Mx, My, Mz;
        magCalibrated(mx, my, mz, Mx, My, Mz);
        float h = tiltHeading(ax / 16384.0f, ay / 16384.0f, Mx, My, Mz);

        // AVR snprintf has no %f -> whole numbers only
        snprintf(line, sizeof(line), "h=%3d  raw m=%5d %5d %5d  a=%6d %6d",
                 (int)(h + 0.5f), mx, my, mz, ax, ay);
        TEST_MESSAGE(line);

        delay(PAUSE_MS);
    }
    TEST_ASSERT_FALSE_MESSAGE(allZero, "compass only gave zeros");
}

void setup()
{
    delay(2000);                       // wait for the Serial monitor
    Wire.begin();                      // this Nano is the I2C master
    Wire.setWireTimeout(3000, true);   // never hang on a stuck bus
    UNITY_BEGIN();
    RUN_TEST(test_wake_up_imu);
    RUN_TEST(test_compass_answers);
    RUN_TEST(test_print_heading);
    UNITY_END();
}

void loop() {}

/* Notes
1) Same register steps as the old firmware (CurrentFirmware):
   initialize() woke the MPU (0x6B = 0x01) with +-2 g (0x1C = 0x00),
   getMotion9() turned bypass on (0x37 = 0x02), then each compass
   reading was: 0x0A = 0x01 (measure once), wait 10 ms, read 6 bytes.

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
   Write down the results, they decide how good the compass mode can be.
*/