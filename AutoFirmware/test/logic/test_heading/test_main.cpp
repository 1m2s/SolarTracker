#include <unity.h>
#include <math.h>
#include "Heading.h"

// tiltHeading(Ax, Ay, Mx, My, Mz) = getTiltHeading() from CurrentFirmware.
// Level board: Ax = Ay = 0.

const float DEG = (float)M_PI / 180.0f;

void setUp(void) {}
void tearDown(void) {}

// Smallest difference between two headings, e.g. 359 and 1 -> 2.
float headingDiff(float a, float b)
{
    float d = fmodf(fabsf(a - b), 360.0f);
    return d > 180.0f ? 360.0f - d : d;
}

void test_four_directions_level(void)
{
    TEST_ASSERT_FLOAT_WITHIN(0.01f,   0.0f, tiltHeading(0, 0,  1,  0, 0));   // +x
    TEST_ASSERT_FLOAT_WITHIN(0.01f,  90.0f, tiltHeading(0, 0,  0, -1, 0));   // -y
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 180.0f, tiltHeading(0, 0, -1,  0, 0));   // -x
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 270.0f, tiltHeading(0, 0,  0,  1, 0));   // +y
}

// Level board: the vertical part Mz must not change the heading.
void test_level_ignores_mz(void)
{
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 90.0f, tiltHeading(0, 0, 0, -1,  0.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 90.0f, tiltHeading(0, 0, 0, -1, 42.0f));
}

// Turn the field once around: every result is in 0..360 and matches.
void test_full_turn(void)
{
    for (int deg = 0; deg < 360; deg++) {
        float Mx =  cosf(deg * DEG);
        float My = -sinf(deg * DEG);
        float h  = tiltHeading(0, 0, Mx, My, 0.3f);
        TEST_ASSERT_TRUE(h >= 0.0f && h < 360.0f);
        TEST_ASSERT_TRUE(headingDiff(h, (float)deg) < 0.01f);
    }
}

// Worked by hand: roll 30 deg (Ay = 0.5), field only along z.
// xh = 0, yh = -sin(30) = -0.5 -> atan2 = -90 -> +360 = 270 -> 360 - 270 = 90
void test_tilt_by_hand(void)
{
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 90.0f, tiltHeading(0, 0.5f, 0, 0, 1));
}

// Noise can push Ax or Ay a bit above 1 g. The old code gave NaN here.
void test_no_nan(void)
{
    TEST_ASSERT_FALSE(isnan(tiltHeading( 0.0f, 1.2f, 1, 0, 0)));
    TEST_ASSERT_FALSE(isnan(tiltHeading(-1.1f, 0.0f, 1, 0, 0)));
    TEST_ASSERT_FALSE(isnan(tiltHeading( 1.0f, 0.0f, 1, 0, 0)));   // 0 / 0 case
}

// CurrentFirmware offsets: Mx - 0, My - 13, Mz + 7. Scale 1200 / 4096.
void test_calibration(void)
{
    float Mx, My, Mz;
    magCalibrated(0, 0, 0, Mx, My, Mz);
    TEST_ASSERT_FLOAT_WITHIN(0.001f,   0.0f, Mx);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, -13.0f, My);
    TEST_ASSERT_FLOAT_WITHIN(0.001f,   7.0f, Mz);

    magCalibrated(4096, 4096, -4096, Mx, My, Mz);
    TEST_ASSERT_FLOAT_WITHIN(0.001f,  1200.0f, Mx);
    TEST_ASSERT_FLOAT_WITHIN(0.001f,  1187.0f, My);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, -1193.0f, Mz);
}

int runAllTests(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_four_directions_level);
    RUN_TEST(test_level_ignores_mz);
    RUN_TEST(test_full_turn);
    RUN_TEST(test_tilt_by_hand);
    RUN_TEST(test_no_nan);
    RUN_TEST(test_calibration);
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