#include <unity.h>
#include <stdint.h>
#include "ClockWatch.h"

// clockFrozen(w, nowMs, rtcSecs): true when the RTC time has not
// changed for at least 2000 ms of millis().

const uint32_t T0 = 830000000UL;     // some RTC time (s since 2000)

ClockWatch w;

void setUp(void)
{
    w.have = false;
    w.secs = 0;
    w.ms   = 0;
}
void tearDown(void) {}

void test_first_reading_is_never_frozen(void)
{
    TEST_ASSERT_FALSE(clockFrozen(w, 0, T0));
}

void test_running_clock(void)
{
    TEST_ASSERT_FALSE(clockFrozen(w, 0,     T0));
    TEST_ASSERT_FALSE(clockFrozen(w, 30000, T0 + 30));   // normal 30 s cycle
    TEST_ASSERT_FALSE(clockFrozen(w, 60000, T0 + 60));
}

void test_stopped_clock(void)
{
    TEST_ASSERT_FALSE(clockFrozen(w, 0,     T0));
    TEST_ASSERT_TRUE (clockFrozen(w, 30000, T0));        // 30 s later, same time
}

// Switch flipped right after a cycle: same second is OK
void test_quick_second_reading(void)
{
    TEST_ASSERT_FALSE(clockFrozen(w, 0,   T0));
    TEST_ASSERT_FALSE(clockFrozen(w, 500, T0));
}

void test_limit_is_2000_ms(void)
{
    TEST_ASSERT_FALSE(clockFrozen(w, 0,    T0));
    TEST_ASSERT_FALSE(clockFrozen(w, 1999, T0));
    TEST_ASSERT_TRUE (clockFrozen(w, 2000, T0));
}

// Many quick readings must not push the 2 s back
void test_repeats_do_not_reset_the_timer(void)
{
    TEST_ASSERT_FALSE(clockFrozen(w, 0,    T0));
    TEST_ASSERT_FALSE(clockFrozen(w, 900,  T0));
    TEST_ASSERT_FALSE(clockFrozen(w, 1800, T0));
    TEST_ASSERT_TRUE (clockFrozen(w, 2700, T0));
}

void test_stays_frozen_then_recovers(void)
{
    TEST_ASSERT_FALSE(clockFrozen(w, 0,     T0));
    TEST_ASSERT_TRUE (clockFrozen(w, 30000, T0));
    TEST_ASSERT_TRUE (clockFrozen(w, 60000, T0));        // still frozen
    TEST_ASSERT_FALSE(clockFrozen(w, 90000, T0 + 5));    // clock moves again
    TEST_ASSERT_FALSE(clockFrozen(w, 120000, T0 + 35));
}

// millis() wraps to 0 after about 49.7 days
void test_across_millis_wrap(void)
{
    const uint32_t before = 4294967290UL;                // 6 ms before the wrap
    TEST_ASSERT_FALSE(clockFrozen(w, before, T0));
    TEST_ASSERT_FALSE(clockFrozen(w, 1993,   T0));       // 1999 ms passed
    TEST_ASSERT_TRUE (clockFrozen(w, 1994,   T0));       // 2000 ms passed
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