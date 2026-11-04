/* Notes
   Hardware test: asks the MPU-9250 (IMU) who it is. It only READS.
   Register WHO_AM_I (0x75) must hold 0x71.
   Needs: tracker on, USB to left Nano, MotorFirmware on the right Nano.
   Run: pio test -e nanotest -f hardware/test_imu_id -v
*/

#include <Arduino.h>
#include <Wire.h>
#include <unity.h>
#include "Imu.h"

uint8_t id = 0;          // the answer
bool    readOk = false;  // did the read work at all?

void setUp(void) {}
void tearDown(void) {}

void test_imu_answers(void)
{
    readOk = imuWhoAmI(id);
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
1) imuWhoAmI() (Imu.cpp) reads register 0x75 in two parts: write the
   register number, then read 1 byte. The write only points at the
   register, it changes nothing in the chip, so this test stays read-only.

2) 0x71 is the fixed ID of the MPU-9250. Similar chips answer
   0x70 (MPU-6500) or 0x73 (MPU-9255). The test prints the value,
   so a wrong chip is easy to spot.
*/