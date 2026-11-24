#include <unity.h>
#include "Controller.h"
#include "config/Board.h"

static const bool SWITCH_OPEN = true;

static const uint32_t NEAR_WRAP_MS = 4294967000u;

static const uint32_t TURN_START_MS = 500u;

static const uint8_t START_AZIMUTH_BYTE   = 90u;
static const uint8_t START_ELEVATION_BYTE = 0u;

static Controller controller;

static void startWithImuOff(void);

static uint32_t startWithImuOnUntilAim(void);

void setUp(void)
{
    ctrlInit(controller, SWITCH_OPEN, 0u);
}

void tearDown(void) {}

static void startWithImuOff(void)
{
    TEST_ASSERT_EQUAL(RESULT_LATCHED, ctrlStatus(controller, true, 0u, 0u));
    TEST_ASSERT_EQUAL(STEP_DONE, ctrlStartupStep(controller, 0u));
}

static uint32_t startWithImuOnUntilAim(void)
{
    TEST_ASSERT_EQUAL(RESULT_LATCHED, ctrlStatus(controller, true, STATUS_IMU_ON, 0u));
    TEST_ASSERT_EQUAL(STEP_SETTLE, ctrlStartupStep(controller, 0u));
    TEST_ASSERT_EQUAL(STEP_START_MEASURE, ctrlStartupStep(controller, ALIGN_SETTLE_MS));
    uint32_t aimMs = ALIGN_SETTLE_MS + ALIGN_MEASURE_MS;
    TEST_ASSERT_EQUAL(STEP_AIM, ctrlStartupStep(controller, aimMs));
    return aimMs;
}

void test_telemetry_not_due_before_interval(void)
{
    TEST_ASSERT_FALSE(ctrlTelemetryDue(controller, 0u));
    TEST_ASSERT_FALSE(ctrlTelemetryDue(controller, TELEMETRY_INTERVAL_MS - 1u));
}

void test_telemetry_due_once_per_interval(void)
{
    TEST_ASSERT_TRUE(ctrlTelemetryDue(controller, TELEMETRY_INTERVAL_MS));
    TEST_ASSERT_FALSE(ctrlTelemetryDue(controller, TELEMETRY_INTERVAL_MS + 1u));
    TEST_ASSERT_FALSE(ctrlTelemetryDue(controller, (2u * TELEMETRY_INTERVAL_MS) - 1u));
    TEST_ASSERT_TRUE(ctrlTelemetryDue(controller, 2u * TELEMETRY_INTERVAL_MS));
}

void test_telemetry_due_across_millis_wrap(void)
{
    ctrlInit(controller, SWITCH_OPEN, NEAR_WRAP_MS);
    TEST_ASSERT_FALSE(ctrlTelemetryDue(controller, NEAR_WRAP_MS + TELEMETRY_INTERVAL_MS - 1u));
    TEST_ASSERT_TRUE(ctrlTelemetryDue(controller, NEAR_WRAP_MS + TELEMETRY_INTERVAL_MS));
}

void test_init_has_start_target(void)
{
    TEST_ASSERT_EQUAL_UINT8(START_AZIMUTH_BYTE, controller.sentAzimuthByte);
    TEST_ASSERT_EQUAL_UINT8(START_ELEVATION_BYTE, controller.sentElevationByte);
    TEST_ASSERT_FALSE(controller.motorStill);
}

void test_target_sent_stores_bytes(void)
{
    ctrlTargetSent(controller, 74u, 22u, 1000u);
    TEST_ASSERT_EQUAL_UINT8(74u, controller.sentAzimuthByte);
    TEST_ASSERT_EQUAL_UINT8(22u, controller.sentElevationByte);
}

void test_still_status_after_start_makes_motor_still(void)
{
    (void)ctrlStatus(controller, true, 0u, TURN_START_MS);
    TEST_ASSERT_TRUE(controller.motorStill);
}

void test_still_status_too_early_is_not_trusted(void)
{
    (void)ctrlStatus(controller, true, 0u, TURN_START_MS - 1u);
    TEST_ASSERT_FALSE(controller.motorStill);
}

void test_moving_status_is_not_still(void)
{
    (void)ctrlStatus(controller, true, 0u, 2000u);
    TEST_ASSERT_TRUE(controller.motorStill);
    (void)ctrlStatus(controller, true, STATUS_MOVING, 4000u);
    TEST_ASSERT_FALSE(controller.motorStill);
}

void test_no_answer_keeps_last_motor_state(void)
{
    (void)ctrlStatus(controller, true, 0u, 2000u);
    TEST_ASSERT_EQUAL(RESULT_NO_ANSWER, ctrlStatus(controller, false, 0u, 4000u));
    TEST_ASSERT_TRUE(controller.motorStill);
}

void test_new_azimuth_makes_motor_not_still(void)
{
    (void)ctrlStatus(controller, true, 0u, 2000u);
    ctrlTargetSent(controller, 74u, 0u, 3000u);
    TEST_ASSERT_FALSE(controller.motorStill);
}

void test_same_azimuth_keeps_motor_still(void)
{
    (void)ctrlStatus(controller, true, 0u, 2000u);
    ctrlTargetSent(controller, START_AZIMUTH_BYTE, 0u, 3000u);
    TEST_ASSERT_TRUE(controller.motorStill);
}

void test_new_elevation_only_keeps_motor_still(void)
{
    (void)ctrlStatus(controller, true, 0u, 2000u);
    ctrlTargetSent(controller, START_AZIMUTH_BYTE, 30u, 3000u);
    TEST_ASSERT_TRUE(controller.motorStill);
    TEST_ASSERT_EQUAL_UINT8(30u, controller.sentElevationByte);
}

void test_still_status_right_after_new_target_is_not_trusted(void)
{
    (void)ctrlStatus(controller, true, 0u, 2000u);
    ctrlTargetSent(controller, 74u, 0u, 3000u);
    (void)ctrlStatus(controller, true, 0u, 3000u + TURN_START_MS - 1u);
    TEST_ASSERT_FALSE(controller.motorStill);
    (void)ctrlStatus(controller, true, 0u, 3000u + TURN_START_MS);
    TEST_ASSERT_TRUE(controller.motorStill);
}

void test_live_compass_after_start_with_imu_off(void)
{
    startWithImuOff();
    TEST_ASSERT_FALSE(ctrlLiveCompass(controller));
    (void)ctrlStatus(controller, true, 0u, 2000u);
    TEST_ASSERT_TRUE(ctrlLiveCompass(controller));
}

void test_no_live_compass_while_motor_moves(void)
{
    startWithImuOff();
    (void)ctrlStatus(controller, true, 0u, 2000u);
    ctrlTargetSent(controller, 74u, 0u, 3000u);
    TEST_ASSERT_FALSE(ctrlLiveCompass(controller));
    (void)ctrlStatus(controller, true, STATUS_MOVING, 5000u);
    TEST_ASSERT_FALSE(ctrlLiveCompass(controller));
    (void)ctrlStatus(controller, true, 0u, 7000u);
    TEST_ASSERT_TRUE(ctrlLiveCompass(controller));
}

void test_no_live_compass_during_startup(void)
{
    uint32_t aimMs = startWithImuOnUntilAim();
    (void)ctrlStatus(controller, true, STATUS_IMU_ON, aimMs);
    TEST_ASSERT_TRUE(controller.motorStill);
    TEST_ASSERT_FALSE(ctrlLiveCompass(controller));
}

void test_no_live_compass_after_fault(void)
{
    startWithImuOff();
    (void)ctrlStatus(controller, true, 0u, 2000u);
    TEST_ASSERT_TRUE(ctrlLiveCompass(controller));
    TEST_ASSERT_EQUAL(RESULT_FAULT, ctrlStatus(controller, true, STATUS_IMU_ON, 4000u));
    TEST_ASSERT_FALSE(ctrlLiveCompass(controller));
}

void test_panel_not_known_with_imu_off(void)
{
    startWithImuOff();
    TEST_ASSERT_FALSE(ctrlPanelKnown(controller));
}

void test_panel_not_known_before_measurement(void)
{
    (void)startWithImuOnUntilAim();
    TEST_ASSERT_FALSE(ctrlPanelKnown(controller));
}

void test_panel_known_after_measurement(void)
{
    uint32_t aimMs = startWithImuOnUntilAim();
    TEST_ASSERT_EQUAL(FAULT_NONE, ctrlStartupMeasured(controller, true, true, 211.2f, aimMs));
    TEST_ASSERT_TRUE(ctrlPanelKnown(controller));
}

void test_panel_not_known_after_failed_measurement(void)
{
    uint32_t aimMs = startWithImuOnUntilAim();
    TEST_ASSERT_EQUAL(FAULT_COMPASS, ctrlStartupMeasured(controller, false, true, 0.0f, aimMs));
    TEST_ASSERT_FALSE(ctrlPanelKnown(controller));
}

int runAllTests(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_telemetry_not_due_before_interval);
    RUN_TEST(test_telemetry_due_once_per_interval);
    RUN_TEST(test_telemetry_due_across_millis_wrap);
    RUN_TEST(test_init_has_start_target);
    RUN_TEST(test_target_sent_stores_bytes);
    RUN_TEST(test_still_status_after_start_makes_motor_still);
    RUN_TEST(test_still_status_too_early_is_not_trusted);
    RUN_TEST(test_moving_status_is_not_still);
    RUN_TEST(test_no_answer_keeps_last_motor_state);
    RUN_TEST(test_new_azimuth_makes_motor_not_still);
    RUN_TEST(test_same_azimuth_keeps_motor_still);
    RUN_TEST(test_new_elevation_only_keeps_motor_still);
    RUN_TEST(test_still_status_right_after_new_target_is_not_trusted);
    RUN_TEST(test_live_compass_after_start_with_imu_off);
    RUN_TEST(test_no_live_compass_while_motor_moves);
    RUN_TEST(test_no_live_compass_during_startup);
    RUN_TEST(test_no_live_compass_after_fault);
    RUN_TEST(test_panel_not_known_with_imu_off);
    RUN_TEST(test_panel_not_known_before_measurement);
    RUN_TEST(test_panel_known_after_measurement);
    RUN_TEST(test_panel_not_known_after_failed_measurement);
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