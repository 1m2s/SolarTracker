#include <unity.h>
#include "Controller.h"
#include "config/Board.h"

static const bool SWITCH_OPEN = true;

static const uint32_t NEAR_WRAP_MS = 4294967000u;

static const uint32_t TURN_START_MS = 500u;

static Controller controller;

static uint32_t latchAndSettle(uint32_t startMs);

static uint32_t measureUntilAim(uint32_t settleMs);

static uint32_t turnUntilHold(uint32_t aimMs);

void setUp(void)
{
    ctrlInit(controller, SWITCH_OPEN, 0u);
}

void tearDown(void) {}

static uint32_t latchAndSettle(uint32_t startMs)
{
    ctrlInit(controller, SWITCH_OPEN, startMs);
    TEST_ASSERT_EQUAL(RESULT_LATCHED, ctrlStatus(controller, true, STATUS_IMU_ON, startMs));
    TEST_ASSERT_EQUAL(STEP_SETTLE, ctrlStartupStep(controller, startMs));
    return startMs;
}

static uint32_t measureUntilAim(uint32_t settleMs)
{
    uint32_t measureMs = settleMs + ALIGN_SETTLE_MS;
    TEST_ASSERT_EQUAL(STEP_START_MEASURE, ctrlStartupStep(controller, measureMs));
    uint32_t aimMs = measureMs + ALIGN_MEASURE_MS;
    TEST_ASSERT_EQUAL(STEP_AIM, ctrlStartupStep(controller, aimMs));
    return aimMs;
}

static uint32_t turnUntilHold(uint32_t aimMs)
{
    TEST_ASSERT_EQUAL(FAULT_NONE, ctrlStartupMeasured(controller, true, true, 206.5f, aimMs));
    uint32_t holdMs = aimMs + TURN_START_MS;
    TEST_ASSERT_TRUE(ctrlStartupMotor(controller, STATUS_IMU_ON, holdMs));
    return holdMs;
}

void test_waits_until_switch_known(void)
{
    TEST_ASSERT_EQUAL(STARTUP_WAITING, controller.startup);
    TEST_ASSERT_EQUAL(STEP_NONE, ctrlStartupStep(controller, 1000u));
    TEST_ASSERT_EQUAL(STARTUP_WAITING, controller.startup);
}

void test_aus_skips_to_done(void)
{
    (void)ctrlStatus(controller, true, 0u, 0u);
    TEST_ASSERT_EQUAL(STEP_DONE, ctrlStartupStep(controller, 0u));
    TEST_ASSERT_EQUAL(STARTUP_DONE, controller.startup);
    TEST_ASSERT_FALSE(ctrlBlocked(controller));
    TEST_ASSERT_TRUE(ctrlAutoDue(controller, 0u));
}

void test_an_starts_settling(void)
{
    (void)latchAndSettle(100u);
    TEST_ASSERT_EQUAL(STARTUP_SETTLING, controller.startup);
    TEST_ASSERT_TRUE(ctrlBlocked(controller));
}

void test_settle_lasts_3_s(void)
{
    uint32_t startMs = latchAndSettle(100u);
    TEST_ASSERT_EQUAL(STEP_NONE, ctrlStartupStep(controller, startMs + ALIGN_SETTLE_MS - 1u));
    TEST_ASSERT_EQUAL(STEP_START_MEASURE, ctrlStartupStep(controller, startMs + ALIGN_SETTLE_MS));
    TEST_ASSERT_EQUAL(STARTUP_MEASURING, controller.startup);
}

void test_measure_reads_for_10_s(void)
{
    uint32_t measureMs = latchAndSettle(100u) + ALIGN_SETTLE_MS;
    TEST_ASSERT_EQUAL(STEP_START_MEASURE, ctrlStartupStep(controller, measureMs));
    TEST_ASSERT_EQUAL(STEP_READ, ctrlStartupStep(controller, measureMs + 1u));
    TEST_ASSERT_EQUAL(STEP_READ, ctrlStartupStep(controller, measureMs + ALIGN_MEASURE_MS - 1u));
    TEST_ASSERT_EQUAL(STEP_AIM, ctrlStartupStep(controller, measureMs + ALIGN_MEASURE_MS));
    TEST_ASSERT_EQUAL(STARTUP_AIMING, controller.startup);
    TEST_ASSERT_EQUAL(STEP_NONE, ctrlStartupStep(controller, measureMs + ALIGN_MEASURE_MS + 1u));
}

void test_no_good_reads_faults_compass(void)
{
    uint32_t aimMs = measureUntilAim(latchAndSettle(100u));
    TEST_ASSERT_EQUAL(FAULT_COMPASS, ctrlStartupMeasured(controller, false, true, 0.0f, aimMs));
    TEST_ASSERT_EQUAL(FAULT_COMPASS, controller.fault);
    TEST_ASSERT_TRUE(ctrlBlocked(controller));
}

void test_unsafe_south_faults_not_south(void)
{
    uint32_t aimMs = measureUntilAim(latchAndSettle(100u));
    TEST_ASSERT_EQUAL(FAULT_NOT_SOUTH, ctrlStartupMeasured(controller, true, false, 30.0f, aimMs));
    TEST_ASSERT_EQUAL(FAULT_NOT_SOUTH, controller.fault);
    TEST_ASSERT_TRUE(ctrlBlocked(controller));
}

void test_good_measure_stores_heading_and_turns(void)
{
    uint32_t aimMs = measureUntilAim(latchAndSettle(100u));
    TEST_ASSERT_EQUAL(FAULT_NONE, ctrlStartupMeasured(controller, true, true, 206.5f, aimMs));
    TEST_ASSERT_EQUAL(STARTUP_TURNING, controller.startup);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 206.5f, controller.panelDeg);
    TEST_ASSERT_TRUE(ctrlBlocked(controller));
}

void test_measured_ignored_outside_aiming(void)
{
    uint32_t startMs = latchAndSettle(100u);
    TEST_ASSERT_EQUAL(FAULT_NONE, ctrlStartupMeasured(controller, false, false, 99.0f, startMs));
    TEST_ASSERT_EQUAL(STARTUP_SETTLING, controller.startup);
    TEST_ASSERT_EQUAL(FAULT_NONE, controller.fault);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, controller.panelDeg);
}

void test_turning_waits_while_moving(void)
{
    uint32_t aimMs = measureUntilAim(latchAndSettle(100u));
    (void)ctrlStartupMeasured(controller, true, true, 206.5f, aimMs);
    uint8_t moving = (uint8_t)(STATUS_IMU_ON | STATUS_MOVING);
    TEST_ASSERT_FALSE(ctrlStartupMotor(controller, moving, aimMs + 1000u));
    TEST_ASSERT_FALSE(ctrlStartupMotor(controller, moving, aimMs + 5000u));
    TEST_ASSERT_EQUAL(STARTUP_TURNING, controller.startup);
    TEST_ASSERT_TRUE(ctrlStartupMotor(controller, STATUS_IMU_ON, aimMs + 5250u));
    TEST_ASSERT_EQUAL(STARTUP_HOLDING, controller.startup);
}

void test_still_answer_too_early_is_ignored(void)
{
    uint32_t aimMs = measureUntilAim(latchAndSettle(100u));
    (void)ctrlStartupMeasured(controller, true, true, 206.5f, aimMs);
    TEST_ASSERT_FALSE(ctrlStartupMotor(controller, STATUS_IMU_ON, aimMs + TURN_START_MS - 1u));
    TEST_ASSERT_EQUAL(STARTUP_TURNING, controller.startup);
    TEST_ASSERT_TRUE(ctrlStartupMotor(controller, STATUS_IMU_ON, aimMs + TURN_START_MS));
}

void test_motor_ignored_outside_turning(void)
{
    uint32_t startMs = latchAndSettle(100u);
    TEST_ASSERT_FALSE(ctrlStartupMotor(controller, STATUS_IMU_ON, startMs + 10000u));
    TEST_ASSERT_EQUAL(STARTUP_SETTLING, controller.startup);
}

void test_hold_lasts_3_s_then_done(void)
{
    uint32_t holdMs = turnUntilHold(measureUntilAim(latchAndSettle(100u)));
    TEST_ASSERT_EQUAL(STEP_NONE, ctrlStartupStep(controller, holdMs + ALIGN_HOLD_MS - 1u));
    TEST_ASSERT_TRUE(ctrlBlocked(controller));
    TEST_ASSERT_EQUAL(STEP_DONE, ctrlStartupStep(controller, holdMs + ALIGN_HOLD_MS));
    TEST_ASSERT_EQUAL(STARTUP_DONE, controller.startup);
    TEST_ASSERT_FALSE(ctrlBlocked(controller));
    TEST_ASSERT_EQUAL(STEP_NONE, ctrlStartupStep(controller, holdMs + ALIGN_HOLD_MS + 1u));
}

void test_auto_waits_until_done(void)
{
    uint32_t holdMs = turnUntilHold(measureUntilAim(latchAndSettle(100u)));
    TEST_ASSERT_FALSE(ctrlAutoDue(controller, holdMs + 1u));
    uint32_t doneMs = holdMs + ALIGN_HOLD_MS;
    (void)ctrlStartupStep(controller, doneMs);
    TEST_ASSERT_TRUE(ctrlAutoDue(controller, doneMs));
}

void test_status_asked_fast_only_while_turning(void)
{
    uint32_t aimMs = measureUntilAim(latchAndSettle(100u));
    (void)ctrlStatusDue(controller, aimMs);
    TEST_ASSERT_FALSE(ctrlStatusDue(controller, aimMs + STATUS_RETRY_MS));
    (void)ctrlStartupMeasured(controller, true, true, 206.5f, aimMs);
    TEST_ASSERT_FALSE(ctrlStatusDue(controller, aimMs + STATUS_RETRY_MS - 1u));
    TEST_ASSERT_TRUE(ctrlStatusDue(controller, aimMs + STATUS_RETRY_MS));
    uint32_t holdMs = aimMs + TURN_START_MS;
    TEST_ASSERT_TRUE(ctrlStartupMotor(controller, STATUS_IMU_ON, holdMs));
    (void)ctrlStatusDue(controller, holdMs);
    TEST_ASSERT_FALSE(ctrlStatusDue(controller, holdMs + STATUS_RETRY_MS));
    TEST_ASSERT_TRUE(ctrlStatusDue(controller, holdMs + COMPASS_CHECK_MS));
}

void test_led_off_during_startup(void)
{
    uint32_t startMs = latchAndSettle(0u);
    TEST_ASSERT_FALSE(ctrlLedOn(controller, startMs));
    uint32_t aimMs = measureUntilAim(startMs);
    TEST_ASSERT_FALSE(ctrlLedOn(controller, 4000u));
    (void)ctrlStartupMeasured(controller, true, true, 206.5f, aimMs);
    TEST_ASSERT_FALSE(ctrlLedOn(controller, 14000u));
    TEST_ASSERT_TRUE(ctrlStartupMotor(controller, STATUS_IMU_ON, 16000u));
    TEST_ASSERT_FALSE(ctrlLedOn(controller, 18000u));
    (void)ctrlStartupStep(controller, 19000u);
    TEST_ASSERT_EQUAL(STARTUP_DONE, controller.startup);
    TEST_ASSERT_TRUE(ctrlLedOn(controller, 20000u));
}

void test_led_fast_blink_on_new_faults(void)
{
    uint32_t aimMs = measureUntilAim(latchAndSettle(0u));
    (void)ctrlStartupMeasured(controller, false, true, 0.0f, aimMs);
    TEST_ASSERT_TRUE(ctrlLedOn(controller, 20000u));
    TEST_ASSERT_FALSE(ctrlLedOn(controller, 20200u));

    aimMs = measureUntilAim(latchAndSettle(0u));
    (void)ctrlStartupMeasured(controller, true, false, 30.0f, aimMs);
    TEST_ASSERT_TRUE(ctrlLedOn(controller, 20000u));
    TEST_ASSERT_FALSE(ctrlLedOn(controller, 20200u));
}

void test_startup_faults_are_permanent(void)
{
    uint32_t aimMs = measureUntilAim(latchAndSettle(100u));
    (void)ctrlStartupMeasured(controller, false, true, 0.0f, aimMs);
    TEST_ASSERT_EQUAL(STEP_NONE, ctrlStartupStep(controller, aimMs + 60000u));
    TEST_ASSERT_EQUAL(FAULT_COMPASS, ctrlStartupMeasured(controller, true, true, 206.5f, aimMs + 60000u));
    TEST_ASSERT_FALSE(ctrlStartupMotor(controller, STATUS_IMU_ON, aimMs + 60000u));
    TEST_ASSERT_FALSE(ctrlStatusDue(controller, aimMs + 60000u));
    TEST_ASSERT_FALSE(ctrlAutoDue(controller, aimMs + 60000u));
    TEST_ASSERT_EQUAL(FAULT_COMPASS, controller.fault);
    TEST_ASSERT_EQUAL(STARTUP_AIMING, controller.startup);
}

void test_switch_moved_during_startup_stops_it(void)
{
    uint32_t startMs = latchAndSettle(100u);
    TEST_ASSERT_EQUAL(RESULT_FAULT, ctrlStatus(controller, true, 0u, startMs + 2000u));
    TEST_ASSERT_EQUAL(STEP_NONE, ctrlStartupStep(controller, startMs + ALIGN_SETTLE_MS));
    TEST_ASSERT_EQUAL(STARTUP_SETTLING, controller.startup);
}

void test_init_resets_startup(void)
{
    (void)turnUntilHold(measureUntilAim(latchAndSettle(100u)));
    ctrlInit(controller, SWITCH_OPEN, 0u);
    TEST_ASSERT_EQUAL(STARTUP_WAITING, controller.startup);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, controller.panelDeg);
}

void test_startup_across_millis_wrap(void)
{
    uint32_t startMs = latchAndSettle(NEAR_WRAP_MS);
    uint32_t measureMs = startMs + ALIGN_SETTLE_MS;
    TEST_ASSERT_EQUAL(STEP_NONE, ctrlStartupStep(controller, measureMs - 1u));
    TEST_ASSERT_EQUAL(STEP_START_MEASURE, ctrlStartupStep(controller, measureMs));
    TEST_ASSERT_EQUAL(STEP_READ, ctrlStartupStep(controller, measureMs + ALIGN_MEASURE_MS - 1u));
    uint32_t aimMs = measureMs + ALIGN_MEASURE_MS;
    TEST_ASSERT_EQUAL(STEP_AIM, ctrlStartupStep(controller, aimMs));
    uint32_t holdMs = turnUntilHold(aimMs);
    TEST_ASSERT_EQUAL(STEP_NONE, ctrlStartupStep(controller, holdMs + ALIGN_HOLD_MS - 1u));
    TEST_ASSERT_EQUAL(STEP_DONE, ctrlStartupStep(controller, holdMs + ALIGN_HOLD_MS));
}

void test_turn_too_long_faults(void)
{
    uint32_t aimMs = measureUntilAim(latchAndSettle(100u));
    (void)ctrlStartupMeasured(controller, true, true, 206.5f, aimMs);
    uint8_t moving = (uint8_t)(STATUS_IMU_ON | STATUS_MOVING);
    TEST_ASSERT_FALSE(ctrlStartupMotor(controller, moving, aimMs + 10000u));
    TEST_ASSERT_EQUAL(STEP_NONE, ctrlStartupStep(controller, aimMs + ALIGN_TURN_MAX_MS - 1u));
    TEST_ASSERT_EQUAL(STEP_FAULT, ctrlStartupStep(controller, aimMs + ALIGN_TURN_MAX_MS));
    TEST_ASSERT_EQUAL(FAULT_TURN_TIMEOUT, controller.fault);
    TEST_ASSERT_TRUE(ctrlBlocked(controller));
    TEST_ASSERT_TRUE(ctrlLedOn(controller, 40000u));
    TEST_ASSERT_FALSE(ctrlLedOn(controller, 40200u));
    TEST_ASSERT_FALSE(ctrlStartupMotor(controller, STATUS_IMU_ON, aimMs + ALIGN_TURN_MAX_MS + 1u));
    TEST_ASSERT_EQUAL(STEP_NONE, ctrlStartupStep(controller, aimMs + 60000u));
}

void test_turn_finished_in_time_no_fault(void)
{
    uint32_t aimMs = measureUntilAim(latchAndSettle(100u));
    (void)ctrlStartupMeasured(controller, true, true, 206.5f, aimMs);
    TEST_ASSERT_TRUE(ctrlStartupMotor(controller, STATUS_IMU_ON, aimMs + ALIGN_TURN_MAX_MS - 1u));
    TEST_ASSERT_EQUAL(STEP_NONE, ctrlStartupStep(controller, aimMs + ALIGN_TURN_MAX_MS));
    TEST_ASSERT_EQUAL(FAULT_NONE, controller.fault);
    TEST_ASSERT_EQUAL(STARTUP_HOLDING, controller.startup);
}

int runAllTests(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_waits_until_switch_known);
    RUN_TEST(test_aus_skips_to_done);
    RUN_TEST(test_an_starts_settling);
    RUN_TEST(test_settle_lasts_3_s);
    RUN_TEST(test_measure_reads_for_10_s);
    RUN_TEST(test_no_good_reads_faults_compass);
    RUN_TEST(test_unsafe_south_faults_not_south);
    RUN_TEST(test_good_measure_stores_heading_and_turns);
    RUN_TEST(test_measured_ignored_outside_aiming);
    RUN_TEST(test_turning_waits_while_moving);
    RUN_TEST(test_still_answer_too_early_is_ignored);
    RUN_TEST(test_motor_ignored_outside_turning);
    RUN_TEST(test_hold_lasts_3_s_then_done);
    RUN_TEST(test_auto_waits_until_done);
    RUN_TEST(test_status_asked_fast_only_while_turning);
    RUN_TEST(test_led_off_during_startup);
    RUN_TEST(test_led_fast_blink_on_new_faults);
    RUN_TEST(test_startup_faults_are_permanent);
    RUN_TEST(test_switch_moved_during_startup_stops_it);
    RUN_TEST(test_init_resets_startup);
    RUN_TEST(test_startup_across_millis_wrap);
    RUN_TEST(test_turn_too_long_faults);
    RUN_TEST(test_turn_finished_in_time_no_fault);
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