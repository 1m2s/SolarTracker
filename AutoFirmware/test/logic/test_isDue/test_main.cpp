#include <unity.h>
#include <stdint.h>
#include "isDue.h"

// isDue(now, last, interval) = at least "interval" ms since "last"?
// Same 30 s interval as main.cpp.

const uint32_t INTERVAL = 30000UL;

void setUp(void) {}
void tearDown(void) {}

void test_normal(void)
{
    TEST_ASSERT_FALSE(isDue(29999, 0, INTERVAL));    // 1 ms too early
    TEST_ASSERT_TRUE (isDue(30000, 0, INTERVAL));    // exactly 30 s
    TEST_ASSERT_TRUE (isDue(45000, 0, INTERVAL));    // late is also OK
}

void test_later_start(void)
{
    TEST_ASSERT_FALSE(isDue(129999, 100000, INTERVAL));
    TEST_ASSERT_TRUE (isDue(130000, 100000, INTERVAL));
}

// last = 4294967290 = 6 ms before millis() wraps to 0.
// now = 29993 -> 6 + 29993 = 29999 ms passed -> not yet.
// now = 29994 -> 6 + 29994 = 30000 ms passed -> due.
void test_across_wrap(void)
{
    const uint32_t last = 4294967290UL;
    TEST_ASSERT_FALSE(isDue(29993, last, INTERVAL));
    TEST_ASSERT_TRUE (isDue(29994, last, INTERVAL));
}

// The wrong way: now >= last + interval.
// last + interval wraps to 29994, a small number.
// 1 ms later (now = 4294967291) the wrong check already says "due".
// isDue correctly says "not yet".
void test_wrong_way_fails_at_wrap(void)
{
    const uint32_t last = 4294967290UL;
    const uint32_t now  = 4294967291UL;              // only 1 ms passed
    const uint32_t next = last + INTERVAL;           // wraps to 29994

    TEST_ASSERT_EQUAL_UINT32(29994UL, next);
    TEST_ASSERT_TRUE (now >= next);                  // wrong way: bug
    TEST_ASSERT_FALSE(isDue(now, last, INTERVAL));   // isDue: correct
}

int runAllTests(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_normal);
    RUN_TEST(test_later_start);
    RUN_TEST(test_across_wrap);
    RUN_TEST(test_wrong_way_fails_at_wrap);
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