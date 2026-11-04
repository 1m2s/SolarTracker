#include <unity.h>
#include <math.h>
#include "Heading.h"

const float RAD_PER_DEG = (float)M_PI / 180.0f;

void setUp(void) {}
void tearDown(void) {}

float headingDiff(float firstHeadingDeg, float secondHeadingDeg)
{
    float differenceDeg = fmodf(fabsf(firstHeadingDeg - secondHeadingDeg), 360.0f);
    return differenceDeg > 180.0f ? 360.0f - differenceDeg : differenceDeg;
}

void test_four_directions_level(void)
{
    TEST_ASSERT_FLOAT_WITHIN(0.01f,   0.0f, tiltHeading(0, 0,  1,  0, 0));
    TEST_ASSERT_FLOAT_WITHIN(0.01f,  90.0f, tiltHeading(0, 0,  0, -1, 0));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 180.0f, tiltHeading(0, 0, -1,  0, 0));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 270.0f, tiltHeading(0, 0,  0,  1, 0));
}

void test_level_ignores_magZ(void)
{
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 90.0f, tiltHeading(0, 0, 0, -1,  0.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 90.0f, tiltHeading(0, 0, 0, -1, 42.0f));
}

void test_full_turn(void)
{
    for (int turnDeg = 0; turnDeg < 360; turnDeg++) {
        float magX       =  cosf(turnDeg * RAD_PER_DEG);
        float magY       = -sinf(turnDeg * RAD_PER_DEG);
        float headingDeg = tiltHeading(0, 0, magX, magY, 0.3f);
        TEST_ASSERT_TRUE(headingDeg >= 0.0f && headingDeg < 360.0f);
        TEST_ASSERT_TRUE(headingDiff(headingDeg, (float)turnDeg) < 0.01f);
    }
}

void test_tilt_by_hand(void)
{
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 90.0f, tiltHeading(0, 0.5f, 0, 0, 1));
}

void test_no_nan(void)
{
    TEST_ASSERT_FALSE(isnan(tiltHeading( 0.0f, 1.2f, 1, 0, 0)));
    TEST_ASSERT_FALSE(isnan(tiltHeading(-1.1f, 0.0f, 1, 0, 0)));
    TEST_ASSERT_FALSE(isnan(tiltHeading( 1.0f, 0.0f, 1, 0, 0)));
}

float rawHeading(int16_t magRawX, int16_t magRawY)
{
    float magX, magY, magZ;
    magCalibrated(magRawX, magRawY, 0, magX, magY, magZ);
    return tiltHeading(0, 0, magX, magY, magZ);
}

float rawRadius(int16_t magRawX, int16_t magRawY)
{
    float magX, magY, magZ;
    magCalibrated(magRawX, magRawY, 0, magX, magY, magZ);
    return sqrtf(magX * magX + magY * magY);
}

void test_centre_goes_to_zero(void)
{
    float magX, magY, magZ;
    magCalibrated(7, 48, 0, magX, magY, magZ);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 0.0f, magX);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 0.0f, magY);
}

void test_z_unchanged(void)
{
    float magX, magY, magZ;
    magCalibrated(0, 0, 0, magX, magY, magZ);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 7.0f, magZ);
    magCalibrated(0, 0, -4096, magX, magY, magZ);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, -1193.0f, magZ);
}

void test_real_half_turn(void)
{
    TEST_ASSERT_TRUE(headingDiff(rawHeading(23, 37), rawHeading(-9, 58)) > 175.0f);
}

void test_real_points_on_a_circle(void)
{
    const int16_t points[5][2] = { {23, 38}, {23, 47}, {17, 57}, {6, 63}, {-9, 58} };
    float radiusMin = 1e9f, radiusMax = 0;
    for (int pointIndex = 0; pointIndex < 5; pointIndex++) {
        float radius = rawRadius(points[pointIndex][0], points[pointIndex][1]);
        if (radius < radiusMin) radiusMin = radius;
        if (radius > radiusMax) radiusMax = radius;
    }
    TEST_ASSERT_TRUE((radiusMax - radiusMin) / radiusMin < 0.10f);
}

int runAllTests(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_four_directions_level);
    RUN_TEST(test_level_ignores_magZ);
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