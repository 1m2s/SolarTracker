#include <unity.h>
#include <math.h>
#include "SonnenStand.h"
#include "targetBytes.h"

// Day: same bytes as MATLAB (a8 = uint8(alpha/2), g8 = uint8(gamma)).
// Night or unsafe: park at a8 = 90 (180 deg, south), g8 = 0 (vertical).

const float Breite = 47.81f;
const float Laenge = 9.66f;

void setUp(void) {}
void tearDown(void) {}

// Helper: real sun position for a time (tf = 0, MEZ), then the bytes
bool bytesAt(int Tag, int Monat, int Stunde, int Minute, uint8_t &a8, uint8_t &g8)
{
    float a, g;
    SonnenStand(Breite, Laenge, Tag, Monat, Stunde, Minute, 0, a, g);
    return targetBytes(a, g, a8, g8);
}

// ---------- day ----------

void test_day_27Oct_1200(void)          // alpha 178.5591, gamma 29.6214 (MATLAB)
{
    uint8_t a8, g8;
    TEST_ASSERT_FALSE(targetBytes(178.5591f, 29.6214f, a8, g8));
    TEST_ASSERT_EQUAL_UINT8(89, a8);
    TEST_ASSERT_EQUAL_UINT8(30, g8);
}

// ---------- night ----------

void test_night_28Oct_2330(void)        // alpha 345.24, gamma -54.33
{
    uint8_t a8, g8;
    TEST_ASSERT_TRUE(bytesAt(28, 10, 23, 30, a8, g8));
    TEST_ASSERT_EQUAL_UINT8(90, a8);
    TEST_ASSERT_EQUAL_UINT8(0,  g8);
}

void test_night_29Oct_0000(void)        // alpha 2.14: the dangerous jump
{
    uint8_t a8, g8;
    TEST_ASSERT_TRUE(bytesAt(29, 10, 0, 0, a8, g8));
    TEST_ASSERT_EQUAL_UINT8(90, a8);
    TEST_ASSERT_EQUAL_UINT8(0,  g8);
}

// ---------- sunrise and sunset, 29 Oct ----------
// gamma: 07:05 -0.022, 07:06 +0.135, 17:04 +0.133, 17:05 -0.025

void test_sunrise_29Oct(void)
{
    uint8_t a8, g8;
    TEST_ASSERT_TRUE(bytesAt(29, 10, 7, 5, a8, g8));     // still night
    TEST_ASSERT_FALSE(bytesAt(29, 10, 7, 6, a8, g8));    // day starts
    TEST_ASSERT_EQUAL_UINT8(55, a8);                     // 110.09 / 2
    TEST_ASSERT_EQUAL_UINT8(0,  g8);                     // 0.135 rounds to 0
}

void test_sunset_29Oct(void)
{
    uint8_t a8, g8;
    TEST_ASSERT_FALSE(bytesAt(29, 10, 17, 4, a8, g8));   // still day
    TEST_ASSERT_TRUE(bytesAt(29, 10, 17, 5, a8, g8));    // night starts
    TEST_ASSERT_EQUAL_UINT8(90, a8);
}

// ---------- edge cases ----------

void test_gamma_zero_parks(void)
{
    uint8_t a8, g8;
    TEST_ASSERT_TRUE(targetBytes(180.0f, 0.0f, a8, g8));
    TEST_ASSERT_EQUAL_UINT8(90, a8);
}

void test_nan_parks(void)
{
    uint8_t a8, g8;
    TEST_ASSERT_TRUE(targetBytes(180.0f, NAN, a8, g8));  // gamma broken
    TEST_ASSERT_TRUE(targetBytes(NAN, 20.0f, a8, g8));   // alpha broken
    TEST_ASSERT_EQUAL_UINT8(90, a8);
}

void test_outside_window_parks(void)
{
    uint8_t a8, g8;
    TEST_ASSERT_TRUE(targetBytes(20.0f,  10.0f, a8, g8));   // too far north-east
    TEST_ASSERT_TRUE(targetBytes(340.0f, 10.0f, a8, g8));   // too far north-west
    TEST_ASSERT_FALSE(targetBytes(50.0f, 10.0f, a8, g8));   // edge: still OK
    TEST_ASSERT_FALSE(targetBytes(310.0f, 10.0f, a8, g8));  // edge: still OK
}

// ---------- year scan (PC only, too slow for the Nano) ----------
// Every minute of a year (no leap year, like JahresTag).
// Proves: the target never leaves 50..310 deg (never crosses north),
// and the safe window never fires during a normal day.

#ifndef ARDUINO
void test_year_scan(void)
{
    const int Tage[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    long dayMinutes   = 0;   // minutes following the sun
    long outside      = 0;   // target outside 50..310 deg (must stay 0)
    long windowFired  = 0;   // sun up, but parked by the window (must stay 0)

    for (int Monat = 1; Monat <= 12; Monat++)
    for (int Tag = 1; Tag <= Tage[Monat - 1]; Tag++)
    for (int Stunde = 0; Stunde < 24; Stunde++)
    for (int Minute = 0; Minute < 60; Minute++) {
        float a, g;
        uint8_t a8, g8;
        SonnenStand(Breite, Laenge, Tag, Monat, Stunde, Minute, 0, a, g);
        bool parked = targetBytes(a, g, a8, g8);

        int target = 2 * a8;                     // what the right Nano turns to
        if (target < 50 || target > 310) outside++;
        if (!parked) dayMinutes++;
        if (parked && g > 0.0f) windowFired++;
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
    RUN_TEST(test_gamma_zero_parks);
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