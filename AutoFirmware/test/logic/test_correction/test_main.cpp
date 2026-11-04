#include <unity.h>
#include <math.h>
#include "Correction.h"

const float TOLERANCE_DEG = 0.001f;

void setUp(void) {}
void tearDown(void) {}

void test_arrow_heading_adds_offset_and_declination(void)
{
    TEST_ASSERT_FLOAT_WITHIN(TOLERANCE_DEG, 30.0f, arrowHeading(40.1f));
}

void test_arrow_heading_wraps_below_zero(void)
{
    TEST_ASSERT_FLOAT_WITHIN(TOLERANCE_DEG, 354.9f, arrowHeading(5.0f));
}

void test_example_250_and_30_gives_40(void)
{
    TEST_ASSERT_FLOAT_WITHIN(TOLERANCE_DEG, 40.0f, correctedAzimuth(250.0f, 30.0f));
}

void test_arrow_south_gives_sun_angle(void)
{
    TEST_ASSERT_FLOAT_WITHIN(TOLERANCE_DEG,  50.0f, correctedAzimuth( 50.0f, 180.0f));
    TEST_ASSERT_FLOAT_WITHIN(TOLERANCE_DEG, 180.0f, correctedAzimuth(180.0f, 180.0f));
    TEST_ASSERT_FLOAT_WITHIN(TOLERANCE_DEG, 310.0f, correctedAzimuth(310.0f, 180.0f));
}

void test_wraps_below_zero(void)
{
    TEST_ASSERT_FLOAT_WITHIN(TOLERANCE_DEG, 290.0f, correctedAzimuth(50.0f, 300.0f));
}

void test_wraps_above_360(void)
{
    TEST_ASSERT_FLOAT_WITHIN(TOLERANCE_DEG, 70.0f, correctedAzimuth(310.0f, 60.0f));
}

void test_exactly_360_becomes_0(void)
{
    TEST_ASSERT_FLOAT_WITHIN(TOLERANCE_DEG, 0.0f, correctedAzimuth(180.0f, 0.0f));
}

void test_result_always_0_to_below_360(void)
{
    for (int sunDeg = 0; sunDeg <= 360; sunDeg += 5) {
        for (int arrowDeg = 0; arrowDeg <= 360; arrowDeg += 5) {
            float resultDeg = correctedAzimuth((float)sunDeg, (float)arrowDeg);
            TEST_ASSERT_TRUE(resultDeg >= 0.0f);
            TEST_ASSERT_TRUE(resultDeg < 360.0f);
        }
    }
}

void test_nan_heading_gives_nan(void)
{
    TEST_ASSERT_TRUE(isnan(arrowHeading(NAN)));
    TEST_ASSERT_TRUE(isnan(correctedAzimuth(200.0f, NAN)));
}

void test_edge_last_358_new_1_keeps_358(void)
{
    TEST_ASSERT_FLOAT_WITHIN(TOLERANCE_DEG, 358.0f, edgeRule(358.0f, 1.0f));
}

void test_edge_last_358_new_9_keeps_358(void)
{
    TEST_ASSERT_FLOAT_WITHIN(TOLERANCE_DEG, 358.0f, edgeRule(358.0f, 9.0f));
}

void test_edge_last_358_new_10_sends_10(void)
{
    TEST_ASSERT_FLOAT_WITHIN(TOLERANCE_DEG, 10.0f, edgeRule(358.0f, 10.0f));
}

void test_edge_last_2_new_359_keeps_2(void)
{
    TEST_ASSERT_FLOAT_WITHIN(TOLERANCE_DEG, 2.0f, edgeRule(2.0f, 359.0f));
}

void test_edge_last_2_new_350_sends_350(void)
{
    TEST_ASSERT_FLOAT_WITHIN(TOLERANCE_DEG, 350.0f, edgeRule(2.0f, 350.0f));
}

void test_edge_big_hand_turn_not_near_edge(void)
{
    TEST_ASSERT_FLOAT_WITHIN(TOLERANCE_DEG, 355.0f, edgeRule(100.0f, 355.0f));
}

void test_edge_normal_tracking_unchanged(void)
{
    TEST_ASSERT_FLOAT_WITHIN(TOLERANCE_DEG, 185.0f, edgeRule(180.0f, 185.0f));
    TEST_ASSERT_FLOAT_WITHIN(TOLERANCE_DEG, 355.0f, edgeRule(358.0f, 355.0f));
    TEST_ASSERT_FLOAT_WITHIN(TOLERANCE_DEG,   3.0f, edgeRule(1.0f, 3.0f));
}

void test_edge_noise_cannot_flip_back(void)
{
    float lastDeg = edgeRule(358.0f, 10.0f);
    TEST_ASSERT_FLOAT_WITHIN(TOLERANCE_DEG, 10.0f, lastDeg);
    TEST_ASSERT_FALSE(needsResend(lastDeg, edgeRule(lastDeg, 8.0f)));
    TEST_ASSERT_FALSE(needsResend(lastDeg, edgeRule(lastDeg, 12.0f)));
}

void test_edge_noise_before_crossing_sends_nothing(void)
{
    TEST_ASSERT_FALSE(needsResend(358.0f, edgeRule(358.0f, 356.0f)));
    TEST_ASSERT_FALSE(needsResend(358.0f, edgeRule(358.0f, 0.0f)));
    TEST_ASSERT_FALSE(needsResend(358.0f, edgeRule(358.0f, 2.0f)));
}

void test_resend_3_deg_no(void)
{
    TEST_ASSERT_FALSE(needsResend(100.0f, 103.0f));
    TEST_ASSERT_FALSE(needsResend(100.0f, 97.0f));
}

void test_resend_4_deg_yes(void)
{
    TEST_ASSERT_TRUE(needsResend(100.0f, 104.0f));
    TEST_ASSERT_TRUE(needsResend(100.0f, 96.0f));
}

void test_resend_kept_edge_angle_no(void)
{
    TEST_ASSERT_FALSE(needsResend(358.0f, edgeRule(358.0f, 1.0f)));
}

void test_resend_nan_no(void)
{
    TEST_ASSERT_FALSE(needsResend(100.0f, NAN));
}

int runAllTests(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_arrow_heading_adds_offset_and_declination);
    RUN_TEST(test_arrow_heading_wraps_below_zero);
    RUN_TEST(test_example_250_and_30_gives_40);
    RUN_TEST(test_arrow_south_gives_sun_angle);
    RUN_TEST(test_wraps_below_zero);
    RUN_TEST(test_wraps_above_360);
    RUN_TEST(test_exactly_360_becomes_0);
    RUN_TEST(test_result_always_0_to_below_360);
    RUN_TEST(test_nan_heading_gives_nan);
    RUN_TEST(test_edge_last_358_new_1_keeps_358);
    RUN_TEST(test_edge_last_358_new_9_keeps_358);
    RUN_TEST(test_edge_last_358_new_10_sends_10);
    RUN_TEST(test_edge_last_2_new_359_keeps_2);
    RUN_TEST(test_edge_last_2_new_350_sends_350);
    RUN_TEST(test_edge_big_hand_turn_not_near_edge);
    RUN_TEST(test_edge_normal_tracking_unchanged);
    RUN_TEST(test_edge_noise_cannot_flip_back);
    RUN_TEST(test_edge_noise_before_crossing_sends_nothing);
    RUN_TEST(test_resend_3_deg_no);
    RUN_TEST(test_resend_4_deg_yes);
    RUN_TEST(test_resend_kept_edge_angle_no);
    RUN_TEST(test_resend_nan_no);
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