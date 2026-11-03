#include <unity.h>
#include <math.h>
#include "AngleConversion.h"
#include "SolarTime.h"
#include "SunAngles.h"
#include "SunPosition.h"
 
// Expected values come from MATLAB (see generate_reference.m).
// TOL = 0.001 degrees: far below the motor resolution (2 deg / 1 deg)
// and big enough for small float differences between PC and Nano.
#define TOL 0.001f
 
const float LATITUDE_DEG  = 47.81f;
const float LONGITUDE_DEG = 9.66f;
 
void setUp(void) {}
void tearDown(void) {}
 
// ---------- small functions ----------
 
void test_degToRad(void)
{
    TEST_ASSERT_FLOAT_WITHIN(1e-5f, 3.1415927f, degToRad(180.0f));
}
 
void test_radToDeg(void)
{
    TEST_ASSERT_FLOAT_WITHIN(1e-4f, 57.2957795f, radToDeg(1.0f));
}
 
void test_dayOfYear(void)
{
    TEST_ASSERT_EQUAL_INT(1,   dayOfYear(1, 1));
    TEST_ASSERT_EQUAL_INT(172, dayOfYear(6, 21));
    TEST_ASSERT_EQUAL_INT(300, dayOfYear(10, 27));
    TEST_ASSERT_EQUAL_INT(355, dayOfYear(12, 21));
}
 
void test_cetMinutes(void)
{
    TEST_ASSERT_EQUAL_INT(720, cetMinutes(12, 0, false));   // winter time
    TEST_ASSERT_EQUAL_INT(720, cetMinutes(13, 0, true));    // summer time: 13:00 = 12:00 CET
}
 
void test_meanLocalMinutes(void)
{
    TEST_ASSERT_FLOAT_WITHIN(TOL, 698.64f, meanLocalMinutes(720.0f, LONGITUDE_DEG));
}
 
void test_hourAngle(void)
{
    TEST_ASSERT_FLOAT_WITHIN(TOL, 0.0f,  hourAngle(720.0f));  // noon
    TEST_ASSERT_FLOAT_WITHIN(TOL, 30.0f, hourAngle(600.0f));  // 10:00
}
 
void test_equationOfTime(void)
{
    TEST_ASSERT_FLOAT_WITHIN(TOL, -2.7749f, equationOfTime(0.0f));
    TEST_ASSERT_FLOAT_WITHIN(TOL, 16.2271f, equationOfTime(300.0f / 365.0f * 360.0f));
}
 
void test_declination(void)
{
    TEST_ASSERT_FLOAT_WITHIN(TOL,  23.4437f, declination(172));
    TEST_ASSERT_FLOAT_WITHIN(TOL, -23.4139f, declination(355));
    TEST_ASSERT_FLOAT_WITHIN(TOL, -12.5577f, declination(300));
}
 
void test_elevation_equinox_noon(void)
{
    // Physics check: noon (hour angle = 0), declination = 0 -> 90 - latitude
    TEST_ASSERT_FLOAT_WITHIN(TOL, 90.0f - LATITUDE_DEG, elevation(0.0f, LATITUDE_DEG, 0.0f));
}
 
void test_azimuth_noon(void)
{
    // acos is very sensitive near 1 in float -> looser tolerance (see SunAngles.cpp)
    TEST_ASSERT_FLOAT_WITHIN(0.05f, 180.0f, azimuth(172, LATITUDE_DEG, 720.0f));
}
 
void test_azimuth_zenith_is_nan(void)
{
    // Sun straight overhead (latitude = declination, noon): no azimuth -> NaN -> park
    TEST_ASSERT_TRUE(isnan(azimuth(172, declination(172), 720.0f)));
}
 
// ---------- full chain ----------
 
void check_sunPosition(int day, int month, int hour, int minute, bool summerTime,
                       float expectedAzimuthDeg, float expectedElevationDeg)
{
    float azimuthDeg, elevationDeg;
    sunPosition(LATITUDE_DEG, LONGITUDE_DEG, day, month, hour, minute, summerTime, azimuthDeg, elevationDeg);
    TEST_ASSERT_FLOAT_WITHIN(TOL, expectedAzimuthDeg, azimuthDeg);
    TEST_ASSERT_FLOAT_WITHIN(TOL, expectedElevationDeg, elevationDeg);
}
 
void test_sunPosition_26Oct_1200(void) { check_sunPosition(26, 10, 12, 0, false, 178.5269f, 29.9640f); }
void test_sunPosition_26Oct_1500(void) { check_sunPosition(26, 10, 15, 0, false, 225.4056f, 18.5318f); }
void test_sunPosition_27Oct_1200(void) { check_sunPosition(27, 10, 12, 0, false, 178.5591f, 29.6214f); }
void test_sunPosition_21Jun_1300(void) { check_sunPosition(21,  6, 13, 0, true,  167.3561f, 65.2079f); }
 
int runAllTests(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_degToRad);
    RUN_TEST(test_radToDeg);
    RUN_TEST(test_dayOfYear);
    RUN_TEST(test_cetMinutes);
    RUN_TEST(test_meanLocalMinutes);
    RUN_TEST(test_hourAngle);
    RUN_TEST(test_equationOfTime);
    RUN_TEST(test_declination);
    RUN_TEST(test_elevation_equinox_noon);
    RUN_TEST(test_azimuth_noon);
    RUN_TEST(test_azimuth_zenith_is_nan);
    RUN_TEST(test_sunPosition_26Oct_1200);
    RUN_TEST(test_sunPosition_26Oct_1500);
    RUN_TEST(test_sunPosition_27Oct_1200);
    RUN_TEST(test_sunPosition_21Jun_1300);
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