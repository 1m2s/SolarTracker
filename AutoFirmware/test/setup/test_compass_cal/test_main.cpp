/* Notes
   Setup tool: records raw compass values for the calibration.
   It WRITES temporary settings into the IMU (wake up, bypass, measure).
   Nothing moves, nothing is stored on the Nano.
   What to do: put one straight side of the base against a table edge.
   Hold still. When "TURN NOW" appears, turn the whole tracker slowly
   EXACTLY 180 deg, until the same side lies against the same table edge
   again. Be there before 25 s and hold still until "STOP".
   The values are printed as "CAL x y z" lines. fit_compass.py (same
   folder) computes the calibration for Heading.cpp from 3 such runs.
   Needs: tracker on, USB to left Nano, MotorFirmware on the right Nano.
   Run (from AutoFirmware/), three times with run1, run2, run3:
     pio test -e nanosetup -f setup/test_compass_cal -v | tee test/setup/test_compass_cal/run1.txt
   Then:
     ~/.platformio/penv/bin/python test/setup/test_compass_cal/fit_compass.py test/setup/test_compass_cal/run1.txt test/setup/test_compass_cal/run2.txt test/setup/test_compass_cal/run3.txt
*/

#include <Arduino.h>
#include <Wire.h>
#include <unity.h>

// ---------- MPU-9250 ----------
const uint8_t MPU_ADDR        = 0x68;
const uint8_t REG_PWR_MGMT_1  = 0x6B;
const uint8_t REG_INT_PIN_CFG = 0x37;

// ---------- AK8963 (compass) ----------
const uint8_t MAG_ADDR  = 0x0C;
const uint8_t REG_WIA   = 0x00;
const uint8_t REG_HXL   = 0x03;
const uint8_t REG_CNTL1 = 0x0A;

// ---------- settings ----------
const uint32_t TURN_MS   = 30000;   // 30 s for half a circle
const uint32_t SAMPLE_MS = 200;     // one CAL line every 0.2 s
const uint8_t  AVG       = 4;       // 4 readings per line (less noise)

uint16_t samples = 0;

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

// One compass measurement (same steps as test_compass).
bool readMag(int16_t &mx, int16_t &my, int16_t &mz)
{
    uint8_t b[6];
    if (!writeReg(MAG_ADDR, REG_CNTL1, 0x01)) {
        return false;
    }
    delay(10);
    if (!readRegs(MAG_ADDR, REG_HXL, 6, b)) {
        return false;
    }
    mx = (int16_t)((b[1] << 8) | b[0]);
    my = (int16_t)((b[3] << 8) | b[2]);
    mz = (int16_t)((b[5] << 8) | b[4]);
    return true;
}

void setUp(void) {}
void tearDown(void) {}

// 1) Wake up the IMU and turn bypass on, then check the compass.
void test_wake_up(void)
{
    uint8_t wia = 0;
    TEST_ASSERT_TRUE_MESSAGE(writeReg(MPU_ADDR, REG_PWR_MGMT_1, 0x01), "MPU-9250 does not answer");
    TEST_ASSERT_TRUE(writeReg(MPU_ADDR, REG_INT_PIN_CFG, 0x02));    // bypass on
    delay(10);
    TEST_ASSERT_TRUE_MESSAGE(readRegs(MAG_ADDR, REG_WIA, 1, &wia), "compass (0x0C) does not answer");
    TEST_ASSERT_EQUAL_HEX8(0x48, wia);
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

        long sx = 0, sy = 0, sz = 0;
        for (uint8_t i = 0; i < AVG; i++) {
            int16_t x, y, z;
            TEST_ASSERT_TRUE(readMag(x, y, z));
            sx += x;
            sy += y;
            sz += z;
        }
        snprintf(line, sizeof(line), "CAL %d %d %d",
                 (int)(sx / AVG), (int)(sy / AVG), (int)(sz / AVG));
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

4) AVG = 4: each line is the average of 4 readings (noise about +-3).
*/