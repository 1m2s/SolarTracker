#include <unity.h>
#include "LedPattern.h"

// LED rule (priority order):
// 1) error  -> fast blink, 200 ms on, 200 ms off
// 2) MATLAB -> always on
// 3) AUTO   -> slow blink, 1000 ms on, 1000 ms off

void setUp(void) {}
void tearDown(void) {}

void test_auto_slow_blink(void)
{
    TEST_ASSERT_TRUE (ledOn(MODE_AUTO, false, 0));       // start of second 0: on
    TEST_ASSERT_TRUE (ledOn(MODE_AUTO, false, 999));     // still on
    TEST_ASSERT_FALSE(ledOn(MODE_AUTO, false, 1000));    // second 1: off
    TEST_ASSERT_FALSE(ledOn(MODE_AUTO, false, 1999));
    TEST_ASSERT_TRUE (ledOn(MODE_AUTO, false, 2000));    // second 2: on again
}

void test_matlab_always_on(void)
{
    TEST_ASSERT_TRUE(ledOn(MODE_MATLAB, false, 0));
    TEST_ASSERT_TRUE(ledOn(MODE_MATLAB, false, 1000));   // where AUTO would be off
    TEST_ASSERT_TRUE(ledOn(MODE_MATLAB, false, 1500));
    TEST_ASSERT_TRUE(ledOn(MODE_MATLAB, false, 123456));
}

void test_error_fast_blink(void)
{
    TEST_ASSERT_TRUE (ledOn(MODE_AUTO, true, 0));
    TEST_ASSERT_TRUE (ledOn(MODE_AUTO, true, 199));
    TEST_ASSERT_FALSE(ledOn(MODE_AUTO, true, 200));      // where AUTO OK would be on
    TEST_ASSERT_FALSE(ledOn(MODE_AUTO, true, 399));
    TEST_ASSERT_TRUE (ledOn(MODE_AUTO, true, 400));
}

void test_error_wins_over_matlab(void)
{
    TEST_ASSERT_TRUE (ledOn(MODE_MATLAB, true, 0));
    TEST_ASSERT_FALSE(ledOn(MODE_MATLAB, true, 200));    // MATLAB alone would be on
}

// millis() wraps after 4294967295 ms to 0.
// 4294967295 / 1000 = 4294967 (odd) -> off. Then 0 -> on.
// The last off phase is only 296 ms instead of 1000 ms. Harmless.
void test_millis_wrap(void)
{
    TEST_ASSERT_FALSE(ledOn(MODE_AUTO, false, 4294967000UL));   // last off phase starts
    TEST_ASSERT_FALSE(ledOn(MODE_AUTO, false, 4294967295UL));   // last ms before the wrap
    TEST_ASSERT_TRUE (ledOn(MODE_AUTO, false, 0));              // after the wrap: on
    TEST_ASSERT_TRUE (ledOn(MODE_MATLAB, false, 4294967295UL)); // MATLAB: still just on
}

int runAllTests(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_auto_slow_blink);
    RUN_TEST(test_matlab_always_on);
    RUN_TEST(test_error_fast_blink);
    RUN_TEST(test_error_wins_over_matlab);
    RUN_TEST(test_millis_wrap);
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