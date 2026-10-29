#include <unity.h>
#include <math.h>
#include "toUint8.h"

// Expected values = what MATLAB uint8() gives.
// Check in MATLAB:
// uint8([-5.3 0 0.49 0.5 1.5 254.4 254.5 255 300 NaN 178.5591/2 29.6214])
// -> 0 0 0 1 2 254 255 255 255 0 89 30

void setUp(void) {}
void tearDown(void) {}

void test_negative_is_zero(void)
{
    TEST_ASSERT_EQUAL_UINT8(0, toUint8(-5.3f));    // sun below horizon
    TEST_ASSERT_EQUAL_UINT8(0, toUint8(0.0f));
}

void test_rounding_like_matlab(void)
{
    TEST_ASSERT_EQUAL_UINT8(0,   toUint8(0.49f));
    TEST_ASSERT_EQUAL_UINT8(1,   toUint8(0.5f));     // .5 rounds up
    TEST_ASSERT_EQUAL_UINT8(2,   toUint8(1.5f));
    TEST_ASSERT_EQUAL_UINT8(254, toUint8(254.4f));
    TEST_ASSERT_EQUAL_UINT8(255, toUint8(254.5f));
}

void test_too_big_is_255(void)
{
    TEST_ASSERT_EQUAL_UINT8(255, toUint8(255.0f));
    TEST_ASSERT_EQUAL_UINT8(255, toUint8(300.0f));
}

void test_nan_is_zero(void)
{
    TEST_ASSERT_EQUAL_UINT8(0, toUint8(NAN));
}

// 27 Oct 12:00 MEZ: alpha 178.5591, gamma 29.6214 (from generate_reference.m)
void test_27Oct_1200(void)
{
    TEST_ASSERT_EQUAL_UINT8(89, toUint8(178.5591f / 2.0f));
    TEST_ASSERT_EQUAL_UINT8(30, toUint8(29.6214f));
}

int runAllTests(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_negative_is_zero);
    RUN_TEST(test_rounding_like_matlab);
    RUN_TEST(test_too_big_is_255);
    RUN_TEST(test_nan_is_zero);
    RUN_TEST(test_27Oct_1200);
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