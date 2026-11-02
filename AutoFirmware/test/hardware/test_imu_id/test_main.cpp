/* Notes
   Hardware test: asks the MPU-9250 (IMU) who it is. It only READS.
   Register WHO_AM_I (0x75) must hold 0x71.
   Needs: tracker on, USB to left Nano, MotorFirmware on the right Nano.
   Run: pio test -e nanotest -f hardware/test_imu_id -v
*/

#include <Arduino.h>
#include <Wire.h>
#include <unity.h>

const uint8_t MPU_ADDR     = 0x68;   // MPU-9250
const uint8_t REG_WHO_AM_I = 0x75;

uint8_t id = 0;          // the answer
bool    readOk = false;  // did the read work at all?

// Reads one register: first say which one, then ask for 1 byte.
bool readReg(uint8_t dev, uint8_t reg, uint8_t &value)
{
    Wire.beginTransmission(dev);
    Wire.write(reg);
    if (Wire.endTransmission() != 0) {
        return false;                  // nobody answered
    }
    if (Wire.requestFrom(dev, (uint8_t)1) != 1) {
        return false;                  // no byte came back
    }
    value = Wire.read();
    return true;
}

void setUp(void) {}
void tearDown(void) {}

void test_imu_answers(void)
{
    readOk = readReg(MPU_ADDR, REG_WHO_AM_I, id);
    TEST_ASSERT_TRUE_MESSAGE(readOk, "MPU-9250 (0x68) does not answer");
}

void test_imu_is_mpu9250(void)
{
    char line[32];
    snprintf(line, sizeof(line), "WHO_AM_I = 0x%02X", id);
    TEST_MESSAGE(line);
    TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x71, id, "not an MPU-9250");
}

void setup()
{
    delay(2000);                       // wait for the Serial monitor
    Wire.begin();                      // this Nano is the I2C master
    Wire.setWireTimeout(3000, true);   // never hang on a stuck bus
    UNITY_BEGIN();
    RUN_TEST(test_imu_answers);
    RUN_TEST(test_imu_is_mpu9250);
    UNITY_END();
}

void loop() {}

/* Notes
1) Reading a register always has two parts: write the register
   number (0x75), then read 1 byte. The write only points at the
   register, it changes nothing in the chip.

2) 0x71 is the fixed ID of the MPU-9250. Similar chips answer
   0x70 (MPU-6500) or 0x73 (MPU-9255). The test prints the value,
   so a wrong chip is easy to spot.
*/