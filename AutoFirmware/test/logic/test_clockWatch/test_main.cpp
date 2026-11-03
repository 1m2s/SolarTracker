#include <unity.h>
#include <stdint.h>
#include "TimeChecks.h"

const uint32_t RTC_START_SECONDS = 830000000UL;

ClockWatch watch;

void setUp(void)
{
    watch.hasReading  = false;
    watch.rtcSeconds  = 0;
    watch.firstSeenMs = 0;
}
void tearDown(void) {}

void test_first_reading_is_never_frozen(void)
{
    TEST_ASSERT_FALSE(clockFrozen(watch, 0, RTC_START_SECONDS));
}

void test_running_clock(void)
{
    TEST_ASSERT_FALSE(clockFrozen(watch, 0,     RTC_START_SECONDS));
    TEST_ASSERT_FALSE(clockFrozen(watch, 30000, RTC_START_SECONDS + 30));
    TEST_ASSERT_FALSE(clockFrozen(watch, 60000, RTC_START_SECONDS + 60));
}

void test_stopped_clock(void)
{
    TEST_ASSERT_FALSE(clockFrozen(watch, 0,     RTC_START_SECONDS));
    TEST_ASSERT_TRUE (clockFrozen(watch, 30000, RTC_START_SECONDS));
}

void test_quick_second_reading(void)
{
    TEST_ASSERT_FALSE(clockFrozen(watch, 0,   RTC_START_SECONDS));
    TEST_ASSERT_FALSE(clockFrozen(watch, 500, RTC_START_SECONDS));
}

void test_limit_is_2000_ms(void)
{
    TEST_ASSERT_FALSE(clockFrozen(watch, 0,    RTC_START_SECONDS));
    TEST_ASSERT_FALSE(clockFrozen(watch, 1999, RTC_START_SECONDS));
    TEST_ASSERT_TRUE (clockFrozen(watch, 2000, RTC_START_SECONDS));
}

void test_repeats_do_not_reset_the_timer(void)
{
    TEST_ASSERT_FALSE(clockFrozen(watch, 0,    RTC_START_SECONDS));
    TEST_ASSERT_FALSE(clockFrozen(watch, 900,  RTC_START_SECONDS));
    TEST_ASSERT_FALSE(clockFrozen(watch, 1800, RTC_START_SECONDS));
    TEST_ASSERT_TRUE (clockFrozen(watch, 2700, RTC_START_SECONDS));
}

void test_stays_frozen_then_recovers(void)
{
    TEST_ASSERT_FALSE(clockFrozen(watch, 0,     RTC_START_SECONDS));
    TEST_ASSERT_TRUE (clockFrozen(watch, 30000, RTC_START_SECONDS));
    TEST_ASSERT_TRUE (clockFrozen(watch, 60000, RTC_START_SECONDS));
    TEST_ASSERT_FALSE(clockFrozen(watch, 90000, RTC_START_SECONDS + 5));
    TEST_ASSERT_FALSE(clockFrozen(watch, 120000, RTC_START_SECONDS + 35));
}

void test_across_millis_wrap(void)
{
    const uint32_t before = 4294967290UL;
    TEST_ASSERT_FALSE(clockFrozen(watch, before, RTC_START_SECONDS));
    TEST_ASSERT_FALSE(clockFrozen(watch, 1993,   RTC_START_SECONDS));
    TEST_ASSERT_TRUE (clockFrozen(watch, 1994,   RTC_START_SECONDS));
}

int runAllTests(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_first_reading_is_never_frozen);
    RUN_TEST(test_running_clock);
    RUN_TEST(test_stopped_clock);
    RUN_TEST(test_quick_second_reading);
    RUN_TEST(test_limit_is_2000_ms);
    RUN_TEST(test_repeats_do_not_reset_the_timer);
    RUN_TEST(test_stays_frozen_then_recovers);
    RUN_TEST(test_across_millis_wrap);
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