#include <unity.h>
#include <stdint.h>
#include "TimeChecks.h"

const uint32_t INTERVAL = 30000UL;

void setUp(void) {}
void tearDown(void) {}

void test_normal(void)
{
    TEST_ASSERT_FALSE(isDue(29999, 0, INTERVAL));
    TEST_ASSERT_TRUE (isDue(30000, 0, INTERVAL));
    TEST_ASSERT_TRUE (isDue(45000, 0, INTERVAL));
}

void test_later_start(void)
{
    TEST_ASSERT_FALSE(isDue(129999, 100000, INTERVAL));
    TEST_ASSERT_TRUE (isDue(130000, 100000, INTERVAL));
}

void test_across_wrap(void)
{
    const uint32_t last = 4294967290UL;
    TEST_ASSERT_FALSE(isDue(29993, last, INTERVAL));
    TEST_ASSERT_TRUE (isDue(29994, last, INTERVAL));
}

void test_wrong_way_fails_at_wrap(void)
{
    const uint32_t last = 4294967290UL;
    const uint32_t now  = 4294967291UL;
    const uint32_t next = last + INTERVAL;

    TEST_ASSERT_EQUAL_UINT32(29994UL, next);
    TEST_ASSERT_TRUE (now >= next);
    TEST_ASSERT_FALSE(isDue(now, last, INTERVAL));
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