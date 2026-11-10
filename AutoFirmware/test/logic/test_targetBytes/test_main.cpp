#include <unity.h>
#include <math.h>
#include "SunPosition.h"
#include "MotorTarget.h"

const float LATITUDE_DEG  = 47.81f;
const float LONGITUDE_DEG = 9.66f;

void setUp(void) {}
void tearDown(void) {}

bool bytesAt(int day, int month, int hour, int minute, uint8_t &azimuthByte, uint8_t &elevationByte)
{
    float azimuthDeg, elevationDeg;
    sunPosition(LATITUDE_DEG, LONGITUDE_DEG, day, month, hour, minute, false, azimuthDeg, elevationDeg);
    return targetBytes(azimuthDeg, elevationDeg, azimuthByte, elevationByte);
}

void test_day_27Oct_1200(void)
{
    uint8_t azimuthByte, elevationByte;
    TEST_ASSERT_FALSE(targetBytes(178.5591f, 29.6214f, azimuthByte, elevationByte));
    TEST_ASSERT_EQUAL_UINT8(89, azimuthByte);
    TEST_ASSERT_EQUAL_UINT8(30, elevationByte);
}

void test_night_28Oct_2330(void)
{
    uint8_t azimuthByte, elevationByte;
    TEST_ASSERT_TRUE(bytesAt(28, 10, 23, 30, azimuthByte, elevationByte));
    TEST_ASSERT_EQUAL_UINT8(90, azimuthByte);
    TEST_ASSERT_EQUAL_UINT8(0,  elevationByte);
}

void test_night_29Oct_0000(void)
{
    uint8_t azimuthByte, elevationByte;
    TEST_ASSERT_TRUE(bytesAt(29, 10, 0, 0, azimuthByte, elevationByte));
    TEST_ASSERT_EQUAL_UINT8(90, azimuthByte);
    TEST_ASSERT_EQUAL_UINT8(0,  elevationByte);
}

void test_sunrise_29Oct(void)
{
    uint8_t azimuthByte, elevationByte;
    TEST_ASSERT_TRUE(bytesAt(29, 10, 7, 5, azimuthByte, elevationByte));
    TEST_ASSERT_FALSE(bytesAt(29, 10, 7, 6, azimuthByte, elevationByte));
    TEST_ASSERT_EQUAL_UINT8(55, azimuthByte);
    TEST_ASSERT_EQUAL_UINT8(0,  elevationByte);
}

void test_sunset_29Oct(void)
{
    uint8_t azimuthByte, elevationByte;
    TEST_ASSERT_FALSE(bytesAt(29, 10, 17, 4, azimuthByte, elevationByte));
    TEST_ASSERT_TRUE(bytesAt(29, 10, 17, 5, azimuthByte, elevationByte));
    TEST_ASSERT_EQUAL_UINT8(90, azimuthByte);
}

void test_elevation_zero_parks(void)
{
    uint8_t azimuthByte, elevationByte;
    TEST_ASSERT_TRUE(targetBytes(180.0f, 0.0f, azimuthByte, elevationByte));
    TEST_ASSERT_EQUAL_UINT8(90, azimuthByte);
}

void test_nan_parks(void)
{
    uint8_t azimuthByte, elevationByte;
    TEST_ASSERT_TRUE(targetBytes(180.0f, NAN, azimuthByte, elevationByte));
    TEST_ASSERT_TRUE(targetBytes(NAN, 20.0f, azimuthByte, elevationByte));
    TEST_ASSERT_EQUAL_UINT8(90, azimuthByte);
}

void test_outside_window_parks(void)
{
    uint8_t azimuthByte, elevationByte;
    TEST_ASSERT_TRUE(targetBytes(20.0f,  10.0f, azimuthByte, elevationByte));
    TEST_ASSERT_TRUE(targetBytes(340.0f, 10.0f, azimuthByte, elevationByte));
    TEST_ASSERT_FALSE(targetBytes(50.0f, 10.0f, azimuthByte, elevationByte));
    TEST_ASSERT_FALSE(targetBytes(310.0f, 10.0f, azimuthByte, elevationByte));
}

void test_align_south_target(void)
{
    uint8_t azimuthByte, elevationByte;
    TEST_ASSERT_FALSE(alignBytes(180.0f, azimuthByte, elevationByte));
    TEST_ASSERT_EQUAL_UINT8(90, azimuthByte);
    TEST_ASSERT_EQUAL_UINT8(0,  elevationByte);
}

void test_align_outdoor_position_2(void)
{
    uint8_t azimuthByte, elevationByte;
    TEST_ASSERT_FALSE(alignBytes(153.5f, azimuthByte, elevationByte));
    TEST_ASSERT_EQUAL_UINT8(77, azimuthByte);
    TEST_ASSERT_EQUAL_UINT8(0,  elevationByte);
}

void test_align_safe_range_edges(void)
{
    uint8_t azimuthByte, elevationByte;
    TEST_ASSERT_FALSE(alignBytes(50.0f, azimuthByte, elevationByte));
    TEST_ASSERT_EQUAL_UINT8(25, azimuthByte);
    TEST_ASSERT_FALSE(alignBytes(310.0f, azimuthByte, elevationByte));
    TEST_ASSERT_EQUAL_UINT8(155, azimuthByte);
}

void test_align_outside_range_parks(void)
{
    uint8_t azimuthByte, elevationByte;
    TEST_ASSERT_TRUE(alignBytes(49.9f, azimuthByte, elevationByte));
    TEST_ASSERT_EQUAL_UINT8(90, azimuthByte);
    TEST_ASSERT_EQUAL_UINT8(0,  elevationByte);
    TEST_ASSERT_TRUE(alignBytes(310.1f, azimuthByte, elevationByte));
    TEST_ASSERT_EQUAL_UINT8(90, azimuthByte);
    TEST_ASSERT_TRUE(alignBytes(330.0f, azimuthByte, elevationByte));
    TEST_ASSERT_EQUAL_UINT8(90, azimuthByte);
}

void test_align_nan_parks(void)
{
    uint8_t azimuthByte, elevationByte;
    TEST_ASSERT_TRUE(alignBytes(NAN, azimuthByte, elevationByte));
    TEST_ASSERT_EQUAL_UINT8(90, azimuthByte);
    TEST_ASSERT_EQUAL_UINT8(0,  elevationByte);
}

#ifndef ARDUINO
void test_year_scan(void)
{
    const int daysInMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    long dayMinutes   = 0;
    long outside      = 0;
    long windowFired  = 0;

    for (int month = 1; month <= 12; month++)
    for (int day = 1; day <= daysInMonth[month - 1]; day++)
    for (int hour = 0; hour < 24; hour++)
    for (int minute = 0; minute < 60; minute++) {
        float azimuthDeg, elevationDeg;
        uint8_t azimuthByte, elevationByte;
        sunPosition(LATITUDE_DEG, LONGITUDE_DEG, day, month, hour, minute, false, azimuthDeg, elevationDeg);
        bool parked = targetBytes(azimuthDeg, elevationDeg, azimuthByte, elevationByte);

        int target = 2 * azimuthByte;
        if (target < 50 || target > 310) outside++;
        if (!parked) dayMinutes++;
        if (parked && elevationDeg > 0.0f) windowFired++;
    }

    TEST_ASSERT_TRUE(dayMinutes > 200000);
    TEST_ASSERT_EQUAL_INT32(0, outside);
    TEST_ASSERT_EQUAL_INT32(0, windowFired);
}
#endif

int runAllTests(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_day_27Oct_1200);
    RUN_TEST(test_night_28Oct_2330);
    RUN_TEST(test_night_29Oct_0000);
    RUN_TEST(test_sunrise_29Oct);
    RUN_TEST(test_sunset_29Oct);
    RUN_TEST(test_elevation_zero_parks);
    RUN_TEST(test_nan_parks);
    RUN_TEST(test_outside_window_parks);
    RUN_TEST(test_align_south_target);
    RUN_TEST(test_align_outdoor_position_2);
    RUN_TEST(test_align_safe_range_edges);
    RUN_TEST(test_align_outside_range_parks);
    RUN_TEST(test_align_nan_parks);
#ifndef ARDUINO
    RUN_TEST(test_year_scan);
#endif
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