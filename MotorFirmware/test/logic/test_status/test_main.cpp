#include <unity.h>
#include <stdint.h>
#include "Status.h"

void setUp(void) {}
void tearDown(void) {}

void test_switch_aus_still_is_0(void)
{
    TEST_ASSERT_EQUAL_UINT8(0u, statusByte(false, false, false));
}

void test_switch_an_still_is_1(void)
{
    TEST_ASSERT_EQUAL_UINT8(1u, statusByte(true, false, false));
}

void test_switch_aus_moving_is_2(void)
{
    TEST_ASSERT_EQUAL_UINT8(2u, statusByte(false, true, false));
}

void test_switch_an_moving_is_3(void)
{
    TEST_ASSERT_EQUAL_UINT8(3u, statusByte(true, true, false));
}

void test_readings_ready_is_4(void)
{
    TEST_ASSERT_EQUAL_UINT8(4u, statusByte(false, false, true));
}

void test_everything_is_7(void)
{
    TEST_ASSERT_EQUAL_UINT8(7u, statusByte(true, true, true));
}

void test_mask_reads_switch_while_moving(void)
{
    TEST_ASSERT_EQUAL_UINT8(STATUS_IMU_ON, statusByte(true, true, false) & STATUS_IMU_ON);
    TEST_ASSERT_EQUAL_UINT8(0u, statusByte(false, true, false) & STATUS_IMU_ON);
}

void test_new_bit_leaves_switch_and_moving_alone(void)
{
    for (uint8_t i = 0u; i < 4u; i++) {
        bool imuOn  = (i & 1u) != 0u;
        bool moving = (i & 2u) != 0u;
        uint8_t without = statusByte(imuOn, moving, false);
        uint8_t with    = statusByte(imuOn, moving, true);
        TEST_ASSERT_EQUAL_UINT8(without & STATUS_IMU_ON, with & STATUS_IMU_ON);
        TEST_ASSERT_EQUAL_UINT8(without & STATUS_MOVING, with & STATUS_MOVING);
        TEST_ASSERT_EQUAL_UINT8(STATUS_READINGS, with & STATUS_READINGS);
        TEST_ASSERT_EQUAL_UINT8(0u, without & STATUS_READINGS);
    }
}

int runAllTests(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_switch_aus_still_is_0);
    RUN_TEST(test_switch_an_still_is_1);
    RUN_TEST(test_switch_aus_moving_is_2);
    RUN_TEST(test_switch_an_moving_is_3);
    RUN_TEST(test_readings_ready_is_4);
    RUN_TEST(test_everything_is_7);
    RUN_TEST(test_mask_reads_switch_while_moving);
    RUN_TEST(test_new_bit_leaves_switch_and_moving_alone);
    return UNITY_END();
}

#ifdef ARDUINO
#include <Arduino.h>
void setup()
{
    delay(2000);
    runAllTests();
}
void loop() {}
#else
int main(void)
{
    return runAllTests();
}
#endif