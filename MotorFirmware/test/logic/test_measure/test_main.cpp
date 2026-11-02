#include <unity.h>
#include "Measure.h"

// measure(raw0, raw1, Up, Pp): analog readings -> volts and watts.
// 1023 = 5 V at the pin.

void setUp(void) {}
void tearDown(void) {}

void test_zero(void)
{
    float Up, Pp;
    measure(0, 0, Up, Pp);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, Up);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, Pp);
}

// Documents the integer-division bug (Measure.cpp, note 2):
// 5 V at A0 gives 25 V, not 5 * 5.545 = 27.7 V.
void test_factor_is_5_not_5_545(void)
{
    float Up, Pp;
    measure(1023, 0, Up, Pp);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 25.0f, Up);
}

// 2.4 V at A0 -> Up = 12 V. 0.5 V at A1 -> I = 1.5 A -> P = 18 W.
void test_power(void)
{
    float Up, Pp;
    int raw0 = 491;     // 491 * 5 / 1023 = 2.3998 V
    int raw1 = 102;     // 102 * 5 / 1023 = 0.4985 V
    measure(raw0, raw1, Up, Pp);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 11.999f, Up);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 17.946f, Pp);
}

int runAllTests(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_zero);
    RUN_TEST(test_factor_is_5_not_5_545);
    RUN_TEST(test_power);
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