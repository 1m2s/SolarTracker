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

// Heading of a level board straight from raw compass values.
float rawHeading(int16_t mx, int16_t my)
{
    float Mx, My, Mz;
    magCalibrated(mx, my, 0, Mx, My, Mz);
    return tiltHeading(0, 0, Mx, My, Mz);
}

// Length of the calibrated (x, y) field.
float rawRadius(int16_t mx, int16_t my)
{
    float Mx, My, Mz;
    magCalibrated(mx, my, 0, Mx, My, Mz);
    return sqrtf(Mx * Mx + My * My);
}

// The centre of the calibration runs (raw 7, 48) must land near 0.
void test_centre_goes_to_zero(void)
{
    float Mx, My, Mz;
    magCalibrated(7, 48, 0, Mx, My, Mz);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 0.0f, Mx);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 0.0f, My);
}

// z is not part of the oval step: still raw * scale + 7.
void test_z_unchanged(void)
{
    float Mx, My, Mz;
    magCalibrated(0, 0, 0, Mx, My, Mz);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 7.0f, Mz);
    magCalibrated(0, 0, -4096, Mx, My, Mz);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, -1193.0f, Mz);
}

// Real data (test_compass_cal, 2 Nov 2026): the two table-edge marks,
// an exact 180 deg turn apart. Raw (23, 37) and (-9, 58).
void test_real_half_turn(void)
{
    TEST_ASSERT_TRUE(headingDiff(rawHeading(23, 37), rawHeading(-9, 58)) > 175.0f);
}

// Real data along the same turn: after the oval step all points
// have about the same distance to the centre (a circle, not an oval).
void test_real_points_on_a_circle(void)
{
    const int16_t pts[5][2] = { {23, 38}, {23, 47}, {17, 57}, {6, 63}, {-9, 58} };
    float rmin = 1e9f, rmax = 0;
    for (int i = 0; i < 5; i++) {
        float r = rawRadius(pts[i][0], pts[i][1]);
        if (r < rmin) rmin = r;
        if (r > rmax) rmax = r;
    }
    TEST_ASSERT_TRUE((rmax - rmin) / rmin < 0.10f);    // within 10 %
}

int runAllTests(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_four_directions_level);
    RUN_TEST(test_level_ignores_mz);
    RUN_TEST(test_full_turn);
    RUN_TEST(test_tilt_by_hand);
    RUN_TEST(test_no_nan);
    RUN_TEST(test_centre_goes_to_zero);
    RUN_TEST(test_z_unchanged);
    RUN_TEST(test_real_half_turn);
    RUN_TEST(test_real_points_on_a_circle);
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