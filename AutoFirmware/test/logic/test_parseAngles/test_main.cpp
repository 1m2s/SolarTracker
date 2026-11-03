#include <unity.h>
#include "MatlabLink.h"
 
// parseAngles reads "azimuth elevation" from MATLAB.
// OK line -> true + values. Broken line -> false, values unchanged.
 
const float TOL = 0.0001f;
 
// Helper: parse a line we expect to be broken
static bool parseOnly(const char *line)
{
    float azimuthDeg = 0.0f, elevationDeg = 0.0f;
    return parseAngles(line, azimuthDeg, elevationDeg);
}
 
void setUp(void) {}
void tearDown(void) {}
 
void test_valid_line(void)
{
    float azimuthDeg = 0.0f, elevationDeg = 0.0f;
    TEST_ASSERT_TRUE(parseAngles("245.41 3.86", azimuthDeg, elevationDeg));
    TEST_ASSERT_FLOAT_WITHIN(TOL, 245.41f, azimuthDeg);
    TEST_ASSERT_FLOAT_WITHIN(TOL, 3.86f, elevationDeg);
}
 
void test_integers(void)
{
    float azimuthDeg = 0.0f, elevationDeg = 0.0f;
    TEST_ASSERT_TRUE(parseAngles("180 0", azimuthDeg, elevationDeg));
    TEST_ASSERT_FLOAT_WITHIN(TOL, 180.0f, azimuthDeg);
    TEST_ASSERT_FLOAT_WITHIN(TOL, 0.0f, elevationDeg);
}
 
// Night: sun below the horizon. Valid line, targetBytes parks it later.
void test_night_is_valid(void)
{
    float azimuthDeg = 0.0f, elevationDeg = 0.0f;
    TEST_ASSERT_TRUE(parseAngles("250.1 -5.2", azimuthDeg, elevationDeg));
    TEST_ASSERT_FLOAT_WITHIN(TOL, 250.1f, azimuthDeg);
    TEST_ASSERT_FLOAT_WITHIN(TOL, -5.2f, elevationDeg);
}
 
void test_edges_are_valid(void)
{
    float azimuthDeg = 1.0f, elevationDeg = 1.0f;
    TEST_ASSERT_TRUE(parseAngles("0 -90", azimuthDeg, elevationDeg));
    TEST_ASSERT_FLOAT_WITHIN(TOL, 0.0f, azimuthDeg);
    TEST_ASSERT_FLOAT_WITHIN(TOL, -90.0f, elevationDeg);
    TEST_ASSERT_TRUE(parseAngles("360 90", azimuthDeg, elevationDeg));
    TEST_ASSERT_FLOAT_WITHIN(TOL, 360.0f, azimuthDeg);
    TEST_ASSERT_FLOAT_WITHIN(TOL, 90.0f, elevationDeg);
}
 
void test_whitespace_ok(void)
{
    float azimuthDeg = 0.0f, elevationDeg = 0.0f;
    TEST_ASSERT_TRUE(parseAngles("245.41 3.86\r", azimuthDeg, elevationDeg));        // Windows line end
    TEST_ASSERT_TRUE(parseAngles("245.41 3.86\n", azimuthDeg, elevationDeg));        // newline left in
    TEST_ASSERT_TRUE(parseAngles("  245.41   3.86  ", azimuthDeg, elevationDeg));    // extra spaces
    TEST_ASSERT_TRUE(parseAngles("245.41\t3.86", azimuthDeg, elevationDeg));         // tab
    TEST_ASSERT_FLOAT_WITHIN(TOL, 245.41f, azimuthDeg);
    TEST_ASSERT_FLOAT_WITHIN(TOL, 3.86f, elevationDeg);
}
 
void test_garbage(void)
{
    TEST_ASSERT_FALSE(parseOnly(""));               // empty
    TEST_ASSERT_FALSE(parseOnly("   "));            // only spaces
    TEST_ASSERT_FALSE(parseOnly("hello"));
    TEST_ASSERT_FALSE(parseOnly("245.41 3.86x"));   // junk at the end
    TEST_ASSERT_FALSE(parseOnly("245,41 3,86"));    // German comma
    TEST_ASSERT_FALSE(parseOnly("245.413.86"));     // space missing
}
 
void test_half_line(void)
{
    TEST_ASSERT_FALSE(parseOnly("245.41"));         // second number missing
    TEST_ASSERT_FALSE(parseOnly("245.41 "));
    TEST_ASSERT_FALSE(parseOnly("245.41 3.86 7"));  // one number too many
}
 
void test_out_of_range(void)
{
    TEST_ASSERT_FALSE(parseOnly("400 10"));         // azimuth > 360
    TEST_ASSERT_FALSE(parseOnly("-1 10"));          // azimuth < 0
    TEST_ASSERT_FALSE(parseOnly("245 95"));         // elevation > 90
    TEST_ASSERT_FALSE(parseOnly("245 -91"));        // elevation < -90
    TEST_ASSERT_FALSE(parseOnly("nan 3"));
    TEST_ASSERT_FALSE(parseOnly("inf 3"));
}
 
// A broken line must not change the old values
void test_no_change_on_error(void)
{
    float azimuthDeg = 1.0f, elevationDeg = 2.0f;
    TEST_ASSERT_FALSE(parseAngles("hello", azimuthDeg, elevationDeg));
    TEST_ASSERT_FALSE(parseAngles("400 10", azimuthDeg, elevationDeg));
    TEST_ASSERT_FALSE(parseAngles("245.41", azimuthDeg, elevationDeg));
    TEST_ASSERT_EQUAL_FLOAT(1.0f, azimuthDeg);
    TEST_ASSERT_EQUAL_FLOAT(2.0f, elevationDeg);
}
 
int runAllTests(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_valid_line);
    RUN_TEST(test_integers);
    RUN_TEST(test_night_is_valid);
    RUN_TEST(test_edges_are_valid);
    RUN_TEST(test_whitespace_ok);
    RUN_TEST(test_garbage);
    RUN_TEST(test_half_line);
    RUN_TEST(test_out_of_range);
    RUN_TEST(test_no_change_on_error);
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