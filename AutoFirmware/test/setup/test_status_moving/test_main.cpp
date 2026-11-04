/* Notes
   Checks bit 1 (stepper moving) of the right Nano's status byte.
   It WRITES to the right Nano and MOVES THE MAST, so it lives in setup/.
   Sends azimuth 200 deg (byte 100), asks for the status every 100 ms
   until bit 1 goes 1 and back to 0, then sends 180 deg (byte 90) back.
   Elevation is not touched.
   Needs: MotorFirmware on the right Nano, fresh power-on, panel facing
   south, tracker on, USB to left Nano.
   Run: pio test -e nanosetup -f setup/test_status_moving -v
*/

#include <Arduino.h>
#include <Wire.h>
#include <unity.h>
#include <stdio.h>

const uint8_t SLAVE_ADDR    = 0x19;    // right Nano (A_AIDE)
const uint8_t REG_AZIMUTH   = 254;
const uint8_t STATUS_MOVING = 0x02u;   // bit 1: stepper moving

const uint8_t AZ_BYTE_TEST  = 100;     // 100 * 2 = 200 deg
const uint8_t AZ_BYTE_START = 90;      // 90 * 2 = 180 deg = south (start)

const uint32_t POLL_MS        = 100;
const uint32_t START_LIMIT_MS = 1000;    // bit 1 must turn on within 1 s
const uint32_t STOP_LIMIT_MS  = 10000;   // and off again within 10 s
const uint32_t NEVER          = 0xFFFFFFFFuL;

uint8_t writeRegister(uint8_t reg, uint8_t value)
{
    Wire.beginTransmission(SLAVE_ADDR);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission();
}

// false = the right Nano did not answer
bool readStatus(uint8_t &status)
{
    if (Wire.requestFrom(SLAVE_ADDR, (uint8_t)1) != 1) {
        return false;
    }
    status = Wire.read();
    return true;
}

// Asks every 100 ms until bit 1 equals wantMoving.
// Gives back how many ms that took, or NEVER after limitMs.
uint32_t waitForMoving(bool wantMoving, uint32_t limitMs)
{
    uint32_t startMs = millis();
    while (millis() - startMs <= limitMs) {
        uint8_t status = 0;
        if (readStatus(status)) {
            bool moving = (status & STATUS_MOVING) != 0;
            if (moving == wantMoving) {
                return millis() - startMs;
            }
        }
        delay(POLL_MS);
    }
    return NEVER;
}

void printTime(const char *label, uint32_t ms)
{
    char line[48];
    snprintf(line, sizeof(line), "%s after %lu ms", label, (unsigned long)ms);
    TEST_MESSAGE(line);
}

void setUp(void) {}
void tearDown(void) {}

void test_still_before_send(void)
{
    uint8_t status = 0;
    TEST_ASSERT_TRUE_MESSAGE(readStatus(status), "no answer (MotorFirmware on the right Nano?)");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(0, status & STATUS_MOVING, "moving before any send");
}

void test_moving_after_send(void)
{
    TEST_ASSERT_EQUAL_UINT8(0, writeRegister(REG_AZIMUTH, AZ_BYTE_TEST));
    uint32_t ms = waitForMoving(true, START_LIMIT_MS);
    TEST_ASSERT_TRUE_MESSAGE(ms != NEVER, "bit 1 never turned on");
    printTime("bit 1 = 1", ms);
}

void test_still_after_arrival(void)
{
    uint32_t ms = waitForMoving(false, STOP_LIMIT_MS);
    TEST_ASSERT_TRUE_MESSAGE(ms != NEVER, "bit 1 never turned off");
    printTime("bit 1 = 0", ms);
}

void test_back_to_start(void)
{
    TEST_ASSERT_EQUAL_UINT8(0, writeRegister(REG_AZIMUTH, AZ_BYTE_START));
    TEST_ASSERT_TRUE_MESSAGE(waitForMoving(true, START_LIMIT_MS) != NEVER, "bit 1 never turned on");
    uint32_t ms = waitForMoving(false, STOP_LIMIT_MS);
    TEST_ASSERT_TRUE_MESSAGE(ms != NEVER, "bit 1 never turned off");
    printTime("back at 180 deg, bit 1 = 0", ms);
}

void setup()
{
    delay(2000);         // wait for the Serial monitor
    Wire.begin();        // this Nano is the I2C master
    UNITY_BEGIN();
    RUN_TEST(test_still_before_send);
    RUN_TEST(test_moving_after_send);
    RUN_TEST(test_still_after_arrival);
    RUN_TEST(test_back_to_start);
    UNITY_END();
}

void loop() {}