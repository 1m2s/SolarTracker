#include <unity.h>
#include "grad2rad.h"
#include "rad2grad.h"
#include "JahresTag.h"
#include "MEZ.h"
#include "MOZ.h"
#include "StundenWinkel.h"
#include "Zeitgleichung.h"
#include "Deklination.h"
#include "HoehenWinkel.h"
#include "Azimut.h"
#include "SonnenStand.h"
 
// Expected values come from MATLAB (see generate_reference.m).
// TOL = 0.001 degrees: far below the motor resolution (2 deg / 1 deg)
// and big enough for small float differences between PC and Nano.
#define TOL 0.001f
 
const float Breite = 47.81f;
const float Laenge = 9.66f;
 
void setUp(void) {}
void tearDown(void) {}
 
// ---------- small functions ----------
 
void test_grad2rad(void)
{
    TEST_ASSERT_FLOAT_WITHIN(1e-5f, 3.1415927f, grad2rad(180.0f));
}
 
void test_rad2grad(void)
{
    TEST_ASSERT_FLOAT_WITHIN(1e-4f, 57.2957795f, rad2grad(1.0f));
}
 
void test_JahresTag(void)
{
    TEST_ASSERT_EQUAL_INT(1,   JahresTag(1, 1));
    TEST_ASSERT_EQUAL_INT(172, JahresTag(6, 21));
    TEST_ASSERT_EQUAL_INT(300, JahresTag(10, 27));
    TEST_ASSERT_EQUAL_INT(355, JahresTag(12, 21));
}
 
void test_MEZ(void)
{
    TEST_ASSERT_EQUAL_INT(720, MEZ(12, 0, 0));   // winter time
    TEST_ASSERT_EQUAL_INT(720, MEZ(13, 0, 1));   // summer time: 13:00 = 12:00 MEZ
}
 
void test_MOZ(void)
{
    TEST_ASSERT_FLOAT_WITHIN(TOL, 698.64f, MOZ(720.0f, Laenge));
}
 
void test_StundenWinkel(void)
{
    TEST_ASSERT_FLOAT_WITHIN(TOL, 0.0f,  StundenWinkel(720.0f));  // noon
    TEST_ASSERT_FLOAT_WITHIN(TOL, 30.0f, StundenWinkel(600.0f));  // 10:00
}
 
void test_Zeitgleichung(void)
{
    TEST_ASSERT_FLOAT_WITHIN(TOL, -2.7749f, Zeitgleichung(0.0f));
    TEST_ASSERT_FLOAT_WITHIN(TOL, 16.2271f, Zeitgleichung(300.0f / 365.0f * 360.0f));
}
 
void test_Deklination(void)
{
    TEST_ASSERT_FLOAT_WITHIN(TOL,  23.4437f, Deklination(172));
    TEST_ASSERT_FLOAT_WITHIN(TOL, -23.4139f, Deklination(355));
    TEST_ASSERT_FLOAT_WITHIN(TOL, -12.5577f, Deklination(300));
}
 
void test_HoehenWinkel_equinox_noon(void)
{
    // Physics check: noon (beta = 0), delta = 0 -> 90 - Breite
    TEST_ASSERT_FLOAT_WITHIN(TOL, 90.0f - Breite, HoehenWinkel(0.0f, Breite, 0.0f));
}
 
void test_Azimut_noon(void)
{
    // acos is very sensitive near 1 in float -> looser tolerance (see Azimut.cpp)
    TEST_ASSERT_FLOAT_WITHIN(0.05f, 180.0f, Azimut(172, Breite, 720.0f));
}
 
// ---------- full chain ----------
 
void check_SonnenStand(int Tag, int Monat, int Stunde, int Minute, int tf,
                       float alpha_ml, float gamma_ml)
{
    float alpha, gamma;
    SonnenStand(Breite, Laenge, Tag, Monat, Stunde, Minute, tf, alpha, gamma);
    TEST_ASSERT_FLOAT_WITHIN(TOL, alpha_ml, alpha);
    TEST_ASSERT_FLOAT_WITHIN(TOL, gamma_ml, gamma);
}
 
void test_SonnenStand_26Oct_1200(void) { check_SonnenStand(26, 10, 12, 0, 0, 178.5269f, 29.9640f); }
void test_SonnenStand_26Oct_1500(void) { check_SonnenStand(26, 10, 15, 0, 0, 225.4056f, 18.5318f); }
void test_SonnenStand_27Oct_1200(void) { check_SonnenStand(27, 10, 12, 0, 0, 178.5591f, 29.6214f); }
void test_SonnenStand_21Jun_1300(void) { check_SonnenStand(21,  6, 13, 0, 1, 167.3561f, 65.2079f); }
 
int runAllTests(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_grad2rad);
    RUN_TEST(test_rad2grad);
    RUN_TEST(test_JahresTag);
    RUN_TEST(test_MEZ);
    RUN_TEST(test_MOZ);
    RUN_TEST(test_StundenWinkel);
    RUN_TEST(test_Zeitgleichung);
    RUN_TEST(test_Deklination);
    RUN_TEST(test_HoehenWinkel_equinox_noon);
    RUN_TEST(test_Azimut_noon);
    RUN_TEST(test_SonnenStand_26Oct_1200);
    RUN_TEST(test_SonnenStand_26Oct_1500);
    RUN_TEST(test_SonnenStand_27Oct_1200);
    RUN_TEST(test_SonnenStand_21Jun_1300);
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
 