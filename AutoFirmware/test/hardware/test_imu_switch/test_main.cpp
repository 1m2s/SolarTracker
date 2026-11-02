/* Notes
   Hardware test: asks the right Nano for the IMU switch. It only READS.
   The right Nano answers 1 byte: 1 = AN, 0 = AUS.
   Needs: MotorFirmware on the right Nano, tracker on, USB to left Nano.
   Run it once with the switch AUS, once with AN.
   Run: pio test -e nanotest -f hardware/test_imu_switch -v
*/

#include <Arduino.h>
#include <Wire.h>
#include <unity.h>

const uint8_t SLAVE_ADDR = 0x19;   // right Nano (A_AIDE)

uint8_t got   = 0;      // how many bytes came back
uint8_t value = 255;    // the answer (255 = nothing received)

void setUp(void) {}
void tearDown(void) {}

void test_right_nano_answers_one_byte(void)
{
    got = Wire.requestFrom(SLAVE_ADDR, (uint8_t)1);   // ask for 1 byte
    if (got == 1) {
        value = Wire.read();
    }
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(1, got, "no answer (MotorFirmware on the right Nano?)");
}

void test_answer_is_0_or_1(void)
{
    TEST_ASSERT_TRUE_MESSAGE(value <= 1, "answer is not 0 or 1");
    TEST_MESSAGE(value == 1 ? "IMU switch: AN" : "IMU switch: AUS");
}

void setup()
{
    delay(2000);         // wait for the Serial monitor
    Wire.begin();        // this Nano is the I2C master
    UNITY_BEGIN();
    RUN_TEST(test_right_nano_answers_one_byte);
    RUN_TEST(test_answer_is_0_or_1);
    UNITY_END();
}

void loop() {}