/* Notes
   Hardware test: asks the I2C bus who is there. It only READS.
   Expects 0x19 (right Nano), 0x68 (MPU-9250), 0x77 (BMP280).
   Needs: Power AN, IMU switch AN, USB to left Nano.
   Run: pio test -e nanotest -f hardware/test_i2c_scan */

#include <Arduino.h>
#include <Wire.h>
#include <unity.h>

// Asks one I2C address if someone is there.
bool i2cAnswers(uint8_t addr)
{
    Wire.beginTransmission(addr);
    return Wire.endTransmission() == 0;
}

void setUp(void) {}
void tearDown(void) {}

void test_motor_nano_0x19(void)
{
    TEST_ASSERT_TRUE_MESSAGE(i2cAnswers(0x19), "right Nano (motors) not found");
}

void test_mpu9250_0x68(void)
{
    TEST_ASSERT_TRUE_MESSAGE(i2cAnswers(0x68), "MPU-9250 not found (IMU switch AN?)");
}

void test_bmp280_0x77(void)
{
    TEST_ASSERT_TRUE_MESSAGE(i2cAnswers(0x77), "BMP280 not found (IMU switch AN?)");
}

void setup()
{
    delay(2000);
    Wire.begin();
    UNITY_BEGIN();
    RUN_TEST(test_motor_nano_0x19);
    RUN_TEST(test_mpu9250_0x68);
    RUN_TEST(test_bmp280_0x77);
    UNITY_END();
}

void loop() {}

/* Notes
 * - Hardware test. Only works with the Nano in the tracker.
 * - Before: Power switch AN, IMU switch AN, USB to the left Nano.
 * - i2cAnswers() knocks on one address. true = someone answered.
 * - One test per part. If a part is missing, the test fails
 *   and prints the message (what to check).
 * - delay(2000): wait so the Serial monitor and the parts are ready.
 * - Wire.begin(): this Nano is the I2C master.
 */