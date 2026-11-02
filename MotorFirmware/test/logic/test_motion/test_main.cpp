#include <unity.h>
#include <stdint.h>
#include <math.h>
#include "Motion.h"

// Same rules as setStepper_2() / setServo_2() in CurrentFirmware.
// Start: panel south (180 deg), vertical (0 deg).

const float STEP_DEG = 1.8f;

// Plays the stepper loop until it stops. Counts the steps.
int runStepper(float azi, float &off1)
{
    int steps = 0;
    for (int i = 0; i < 1000; i++) {           // safety limit
        int8_t dir = stepDirection(azi, off1);
        if (dir == 0) {
            break;
        }
        off1 += dir * STEP_DEG;
        steps++;
    }
    return steps;
}

void setUp(void) {}
void tearDown(void) {}

// At power-on the right Nano gets 180 deg -> nothing moves.
void test_start_value_does_not_move(void)
{
    TEST_ASSERT_EQUAL_INT8(0, stepDirection(180.0f, 0.0f));
}

void test_direction(void)
{
    TEST_ASSERT_EQUAL_INT8( 1, stepDirection(182.0f, 0.0f));   // bigger azimuth -> +1
    TEST_ASSERT_EQUAL_INT8(-1, stepDirection(178.0f, 0.0f));   // smaller -> -1
}

// Inside +-0.9 deg: stay. Just outside: step.
void test_hysteresis_edge(void)
{
    TEST_ASSERT_EQUAL_INT8( 0, stepDirection(180.8f, 0.0f));
    TEST_ASSERT_EQUAL_INT8( 0, stepDirection(179.2f, 0.0f));
    TEST_ASSERT_EQUAL_INT8( 1, stepDirection(181.0f, 0.0f));
    TEST_ASSERT_EQUAL_INT8(-1, stepDirection(179.0f, 0.0f));
}

// Example from Phase 2.5b: 258 deg needs 43 steps.
void test_258_needs_43_steps(void)
{
    float off1 = 0;
    TEST_ASSERT_EQUAL_INT(43, runStepper(258.0f, off1));
}

// For every target the left Nano can send (a8 = 0..255 -> 0..510 deg),
// the panel ends within 0.9 deg and never keeps stepping.
void test_all_targets_settle(void)
{
    for (int a8 = 0; a8 <= 255; a8++) {
        float azi  = 2.0f * a8;
        float off1 = 0;
        int steps = runStepper(azi, off1);
        TEST_ASSERT_TRUE(steps < 1000);                          // it stopped
        TEST_ASSERT_TRUE(fabsf(azi - (180.0f + off1)) <= 0.9f + 1e-3f);
    }
}

// Night park: from the sun back to south ends where it started.
void test_back_to_south(void)
{
    float off1 = 0;
    runStepper(258.0f, off1);
    runStepper(180.0f, off1);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, off1);
}

void test_servo_moves_one_degree(void)
{
    TEST_ASSERT_EQUAL_FLOAT( 1.0f, servoStep(30.0f, 0.0f));
    TEST_ASSERT_EQUAL_FLOAT(29.0f, servoStep(0.0f, 30.0f));
    TEST_ASSERT_EQUAL_FLOAT(30.0f, servoStep(30.0f, 30.0f));   // at target: stay
}

void test_servo_reaches_target(void)
{
    float off2 = 0;
    for (int i = 0; i < 29; i++) {
        off2 = servoStep(29.0f, off2);
    }
    TEST_ASSERT_EQUAL_FLOAT(29.0f, off2);
    TEST_ASSERT_EQUAL_FLOAT(29.0f, servoStep(29.0f, off2));    // and stays
}

int runAllTests(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_start_value_does_not_move);
    RUN_TEST(test_direction);
    RUN_TEST(test_hysteresis_edge);
    RUN_TEST(test_258_needs_43_steps);
    RUN_TEST(test_all_targets_settle);
    RUN_TEST(test_back_to_south);
    RUN_TEST(test_servo_moves_one_degree);
    RUN_TEST(test_servo_reaches_target);
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