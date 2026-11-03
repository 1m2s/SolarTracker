#include <unity.h>
#include <math.h>
#include "SunPosition.h"
#include "MotorTarget.h"

// Day: same bytes as MATLAB (azimuthByte = uint8(azimuthDeg / 2),
// elevationByte = uint8(elevationDeg)).
// Night or unsafe: park at azimuthByte = 90 (180 deg, south),
// elevationByte = 0 (vertical).

const float LATITUDE_DEG  = 47.81f;
const float LONGITUDE_DEG = 9.66f;

void setUp(void) {}
void tearDown(void) {}

// Helper: real sun position for a time (summerTimeFlag = 0, CET), then the bytes
bool bytesAt(int day, int month, int hour, int minute, uint8_t &azimuthByte, uint8_t &elevationByte)
{
    float azimuthDeg, elevationDeg;
    sunPosition(LATITUDE_DEG, LONGITUDE_DEG, day, month, hour, minute, 0, azimuthDeg, elevationDeg);
    return targetBytes(azimuthDeg, elevationDeg, azimuthByte, elevationByte);
}

// ---------- day ----------

void test_day_27Oct_1200(void)          // azimuth 178.5591, elevation 29.6214 (MATLAB)
{
    uint8_t azimuthByte, elevationByte;
    TEST_ASSERT_FALSE(targetBytes(178.5591f, 29.6214f, azimuthByte, elevationByte));
    TEST_ASSERT_EQUAL_UINT8(89, azimuthByte);
    TEST_ASSERT_EQUAL_UINT8(30, elevationByte);
}

// ---------- night ----------

void test_night_28Oct_2330(void)        // azimuth 345.24, elevation -54.33
{
    uint8_t azimuthByte, elevationByte;
    TEST_ASSERT_TRUE(bytesAt(28, 10, 23, 30, azimuthByte, elevationByte));
    TEST_ASSERT_EQUAL_UINT8(90, azimuthByte);
    TEST_ASSERT_EQUAL_UINT8(0,  elevationByte);
}

void test_night_29Oct_0000(void)        // azimuth 2.14: the dangerous jump
{
    uint8_t azimuthByte, elevationByte;
    TEST_ASSERT_TRUE(bytesAt(29, 10, 0, 0, azimuthByte, elevationByte));
    TEST_ASSERT_EQUAL_UINT8(90, azimuthByte);
    TEST_ASSERT_EQUAL_UINT8(0,  elevationByte);
}

// ---------- sunrise and sunset, 29 Oct ----------
// elevation: 07:05 -0.022, 07:06 +0.135, 17:04 +0.133, 17:05 -0.025

void test_sunrise_29Oct(void)
{
    uint8_t azimuthByte, elevationByte;
    TEST_ASSERT_TRUE(bytesAt(29, 10, 7, 5, azimuthByte, elevationByte));     // still night
    TEST_ASSERT_FALSE(bytesAt(29, 10, 7, 6, azimuthByte, elevationByte));    // day starts
    TEST_ASSERT_EQUAL_UINT8(55, azimuthByte);                                // 110.09 / 2
    TEST_ASSERT_EQUAL_UINT8(0,  elevationByte);                              // 0.135 rounds to 0
}

void test_sunset_29Oct(void)
{
    uint8_t azimuthByte, elevationByte;
    TEST_ASSERT_FALSE(bytesAt(29, 10, 17, 4, azimuthByte, elevationByte));   // still day
    TEST_ASSERT_TRUE(bytesAt(29, 10, 17, 5, azimuthByte, elevationByte));    // night starts
    TEST_ASSERT_EQUAL_UINT8(90, azimuthByte);
}

// ---------- edge cases ----------

void test_elevation_zero_parks(void)
{
    uint8_t azimuthByte, elevationByte;
    TEST_ASSERT_TRUE(targetBytes(180.0f, 0.0f, azimuthByte, elevationByte));
    TEST_ASSERT_EQUAL_UINT8(90, azimuthByte);
}

void test_nan_parks(void)
{
    uint8_t azimuthByte, elevationByte;
    TEST_ASSERT_TRUE(targetBytes(180.0f, NAN, azimuthByte, elevationByte));  // elevation broken
    TEST_ASSERT_TRUE(targetBytes(NAN, 20.0f, azimuthByte, elevationByte));   // azimuth broken
    TEST_ASSERT_EQUAL_UINT8(90, azimuthByte);
}

void test_outside_window_parks(void)
{
    uint8_t azimuthByte, elevationByte;
    TEST_ASSERT_TRUE(targetBytes(20.0f,  10.0f, azimuthByte, elevationByte));   // too far north-east
    TEST_ASSERT_TRUE(targetBytes(340.0f, 10.0f, azimuthByte, elevationByte));   // too far north-west
    TEST_ASSERT_FALSE(targetBytes(50.0f, 10.0f, azimuthByte, elevationByte));   // edge: still OK
    TEST_ASSERT_FALSE(targetBytes(310.0f, 10.0f, azimuthByte, elevationByte));  // edge: still OK
}

// ---------- year scan (PC only, too slow for the Nano) ----------
// Every minute of a year (no leap year, like dayOfYear).
// Proves: the target never leaves 50..310 deg (never crosses north),
// and the safe window never fires during a normal day.

#ifndef ARDUINO
void test_year_scan(void)
{
    const int daysInMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    long dayMinutes   = 0;   // minutes following the sun
    long outside      = 0;   // target outside 50..310 deg (must stay 0)
    long windowFired  = 0;   // sun up, but parked by the window (must stay 0)

    for (int month = 1; month <= 12; month++)
    for (int day = 1; day <= daysInMonth[month - 1]; day++)
    for (int hour = 0; hour < 24; hour++)
    for (int minute = 0; minute < 60; minute++) {
        float azimuthDeg, elevationDeg;
        uint8_t azimuthByte, elevationByte;
        sunPosition(LATITUDE_DEG, LONGITUDE_DEG, day, month, hour, minute, 0, azimuthDeg, elevationDeg);
        bool parked = targetBytes(azimuthDeg, elevationDeg, azimuthByte, elevationByte);

        int target = 2 * azimuthByte;            // what the right Nano turns to
        if (target < 50 || target > 310) outside++;
        if (!parked) dayMinutes++;
        if (parked && elevationDeg > 0.0f) windowFired++;
    }

    TEST_ASSERT_TRUE(dayMinutes > 200000);       // the scan really saw days
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
#ifndef ARDUINO
    RUN_TEST(test_year_scan);
#endif
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