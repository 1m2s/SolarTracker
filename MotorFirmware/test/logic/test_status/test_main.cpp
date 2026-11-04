#include <unity.h>
#include <stdint.h>
#include "Status.h"

// bit 0 = IMU switch AN, bit 1 = stepper moving.

void setUp(void) {}
void tearDown(void) {}

void test_switch_aus_still_is_0(void)
{
    TEST_ASSERT_EQUAL_UINT8(0u, statusByte(false, false));
}

void test_switch_an_still_is_1(void)
{
    TEST_ASSERT_EQUAL_UINT8(1u, statusByte(true, false));
}

void test_switch_aus_moving_is_2(void)
{
    TEST_ASSERT_EQUAL_UINT8(2u, statusByte(false, true));
}

void test_switch_an_moving_is_3(void)
{
    TEST_ASSERT_EQUAL_UINT8(3u, statusByte(true, true));
}

// The left Nano must still read the switch right while the motor moves.
void test_mask_reads_switch_while_moving(void)
{
    TEST_ASSERT_EQUAL_UINT8(STATUS_IMU_ON, statusByte(true, true) & STATUS_IMU_ON);
    TEST_ASSERT_EQUAL_UINT8(0u, statusByte(false, true) & STATUS_IMU_ON);
}

int runAllTests(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_switch_aus_still_is_0);
    RUN_TEST(test_switch_an_still_is_1);
    RUN_TEST(test_switch_aus_moving_is_2);
    RUN_TEST(test_switch_an_moving_is_3);
    RUN_TEST(test_mask_reads_switch_while_moving);
    return UNITY_END();
}

#ifdef ARDUINO
#include <Arduino.h>
void setup()
{
    delay(2000);       // wait for the Serial monitor
    runAllTests();
}
void loop() {}
#else
int main(void)
{
    return runAllTests();
}
#endif