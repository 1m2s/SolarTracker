#include <unity.h>
#include "Controller.h"
#include "config/Board.h"

static const bool SWITCH_OPEN   = true;
static const bool SWITCH_CLOSED = false;

static const uint32_t NEAR_WRAP_MS = 4294967000u;

static Controller controller;

static uint32_t flipSwitch(bool switchOpen, uint32_t startMs);

void setUp(void)
{
    ctrlInit(controller, SWITCH_OPEN, 0u);
    (void)ctrlStatus(controller, true, 0u, 0u);
}

void tearDown(void) {}

static uint32_t flipSwitch(bool switchOpen, uint32_t startMs)
{
    uint32_t acceptMs = startMs + DEBOUNCE_MS;
    TEST_ASSERT_FALSE(ctrlSwitch(controller, switchOpen, startMs));
    TEST_ASSERT_TRUE(ctrlSwitch(controller, switchOpen, acceptMs));
    return acceptMs;
}

void test_init_takes_mode_from_switch(void)
{
    ctrlInit(controller, SWITCH_OPEN, 0u);
    TEST_ASSERT_EQUAL(MODE_AUTO, controller.mode);
    ctrlInit(controller, SWITCH_CLOSED, 0u);
    TEST_ASSERT_EQUAL(MODE_MATLAB, controller.mode);
    TEST_ASSERT_FALSE(controller.error);
}

void test_same_position_never_changes_mode(void)
{
    for (uint32_t nowMs = 0u; nowMs < 1000u; nowMs += 10u) {
        TEST_ASSERT_FALSE(ctrlSwitch(controller, SWITCH_OPEN, nowMs));
    }
    TEST_ASSERT_EQUAL(MODE_AUTO, controller.mode);
}

void test_debounce_needs_50_ms_still(void)
{
    TEST_ASSERT_FALSE(ctrlSwitch(controller, SWITCH_CLOSED, 100u));
    TEST_ASSERT_FALSE(ctrlSwitch(controller, SWITCH_CLOSED, 149u));
    TEST_ASSERT_EQUAL(MODE_AUTO, controller.mode);
    TEST_ASSERT_TRUE(ctrlSwitch(controller, SWITCH_CLOSED, 150u));
    TEST_ASSERT_EQUAL(MODE_MATLAB, controller.mode);
}

void test_bounce_restarts_debounce(void)
{
    TEST_ASSERT_FALSE(ctrlSwitch(controller, SWITCH_CLOSED, 100u));
    TEST_ASSERT_FALSE(ctrlSwitch(controller, SWITCH_OPEN, 130u));
    TEST_ASSERT_FALSE(ctrlSwitch(controller, SWITCH_CLOSED, 140u));
    TEST_ASSERT_FALSE(ctrlSwitch(controller, SWITCH_CLOSED, 180u));
    TEST_ASSERT_TRUE(ctrlSwitch(controller, SWITCH_CLOSED, 190u));
}

void test_mode_change_reported_once(void)
{
    uint32_t acceptMs = flipSwitch(SWITCH_CLOSED, 100u);
    TEST_ASSERT_FALSE(ctrlSwitch(controller, SWITCH_CLOSED, acceptMs + 1u));
    TEST_ASSERT_FALSE(ctrlSwitch(controller, SWITCH_CLOSED, acceptMs + 5000u));
}

void test_mode_change_clears_error(void)
{
    ctrlAutoResult(controller, false);
    TEST_ASSERT_TRUE(controller.error);
    (void)flipSwitch(SWITCH_CLOSED, 100u);
    TEST_ASSERT_FALSE(controller.error);
}

void test_auto_first_cycle_runs_at_once(void)
{
    TEST_ASSERT_TRUE(ctrlAutoDue(controller, 1000u));
    TEST_ASSERT_FALSE(ctrlAutoDue(controller, 1001u));
}

void test_auto_every_30_s(void)
{
    TEST_ASSERT_TRUE(ctrlAutoDue(controller, 1000u));
    TEST_ASSERT_FALSE(ctrlAutoDue(controller, 1000u + AUTO_INTERVAL_MS - 1u));
    TEST_ASSERT_TRUE(ctrlAutoDue(controller, 1000u + AUTO_INTERVAL_MS));
    TEST_ASSERT_FALSE(ctrlAutoDue(controller, 1000u + AUTO_INTERVAL_MS + 1u));
}

void test_auto_never_due_in_matlab(void)
{
    ctrlInit(controller, SWITCH_CLOSED, 0u);
    TEST_ASSERT_FALSE(ctrlAutoDue(controller, 0u));
    TEST_ASSERT_FALSE(ctrlAutoDue(controller, 10u * AUTO_INTERVAL_MS));
}

void test_back_to_auto_runs_at_once(void)
{
    TEST_ASSERT_TRUE(ctrlAutoDue(controller, 1000u));
    uint32_t matlabMs = flipSwitch(SWITCH_CLOSED, 2000u);
    uint32_t autoMs   = flipSwitch(SWITCH_OPEN, matlabMs + 1000u);
    TEST_ASSERT_TRUE(ctrlAutoDue(controller, autoMs));
}

void test_auto_cycle_across_millis_wrap(void)
{
    TEST_ASSERT_TRUE(ctrlAutoDue(controller, NEAR_WRAP_MS));
    TEST_ASSERT_FALSE(ctrlAutoDue(controller, NEAR_WRAP_MS + AUTO_INTERVAL_MS - 1u));
    TEST_ASSERT_TRUE(ctrlAutoDue(controller, NEAR_WRAP_MS + AUTO_INTERVAL_MS));
}

void test_auto_result_sets_and_clears_error(void)
{
    ctrlAutoResult(controller, false);
    TEST_ASSERT_TRUE(controller.error);
    ctrlAutoResult(controller, true);
    TEST_ASSERT_FALSE(controller.error);
}

void test_matlab_timeout_after_180_s(void)
{
    ctrlInit(controller, SWITCH_CLOSED, 0u);
    TEST_ASSERT_FALSE(ctrlMatlabTimedOut(controller, MATLAB_TIMEOUT_MS - 1u));
    TEST_ASSERT_FALSE(controller.error);
    TEST_ASSERT_TRUE(ctrlMatlabTimedOut(controller, MATLAB_TIMEOUT_MS));
    TEST_ASSERT_TRUE(controller.error);
}

void test_matlab_timeout_reported_once(void)
{
    ctrlInit(controller, SWITCH_CLOSED, 0u);
    TEST_ASSERT_TRUE(ctrlMatlabTimedOut(controller, MATLAB_TIMEOUT_MS));
    TEST_ASSERT_FALSE(ctrlMatlabTimedOut(controller, MATLAB_TIMEOUT_MS + 1u));
    TEST_ASSERT_FALSE(ctrlMatlabTimedOut(controller, 10u * MATLAB_TIMEOUT_MS));
}

void test_no_timeout_in_auto(void)
{
    TEST_ASSERT_FALSE(ctrlMatlabTimedOut(controller, 10u * MATLAB_TIMEOUT_MS));
    TEST_ASSERT_FALSE(controller.error);
}

void test_good_line_restarts_timeout_and_clears_error(void)
{
    ctrlInit(controller, SWITCH_CLOSED, 0u);
    TEST_ASSERT_TRUE(ctrlMatlabTimedOut(controller, MATLAB_TIMEOUT_MS));
    ctrlMatlabLine(controller, 200000u, true);
    TEST_ASSERT_FALSE(controller.error);
    TEST_ASSERT_FALSE(ctrlMatlabTimedOut(controller, 200000u + MATLAB_TIMEOUT_MS - 1u));
    TEST_ASSERT_TRUE(ctrlMatlabTimedOut(controller, 200000u + MATLAB_TIMEOUT_MS));
}

void test_line_not_answered_sets_error(void)
{
    ctrlInit(controller, SWITCH_CLOSED, 0u);
    ctrlMatlabLine(controller, 1000u, false);
    TEST_ASSERT_TRUE(controller.error);
    TEST_ASSERT_FALSE(ctrlMatlabTimedOut(controller, 1000u + MATLAB_TIMEOUT_MS - 1u));
}

void test_switch_to_matlab_restarts_timeout(void)
{
    uint32_t matlabMs = flipSwitch(SWITCH_CLOSED, 500000u);
    TEST_ASSERT_FALSE(ctrlMatlabTimedOut(controller, matlabMs + MATLAB_TIMEOUT_MS - 1u));
    TEST_ASSERT_TRUE(ctrlMatlabTimedOut(controller, matlabMs + MATLAB_TIMEOUT_MS));
}

void test_matlab_timeout_across_millis_wrap(void)
{
    ctrlInit(controller, SWITCH_CLOSED, NEAR_WRAP_MS);
    TEST_ASSERT_FALSE(ctrlMatlabTimedOut(controller, NEAR_WRAP_MS + MATLAB_TIMEOUT_MS - 1u));
    TEST_ASSERT_TRUE(ctrlMatlabTimedOut(controller, NEAR_WRAP_MS + MATLAB_TIMEOUT_MS));
}

void test_led_auto_slow_blink(void)
{
    TEST_ASSERT_TRUE(ctrlLedOn(controller, 0u));
    TEST_ASSERT_TRUE(ctrlLedOn(controller, 999u));
    TEST_ASSERT_FALSE(ctrlLedOn(controller, 1000u));
    TEST_ASSERT_FALSE(ctrlLedOn(controller, 1999u));
    TEST_ASSERT_TRUE(ctrlLedOn(controller, 2000u));
}

void test_led_matlab_always_on(void)
{
    ctrlInit(controller, SWITCH_CLOSED, 0u);
    TEST_ASSERT_TRUE(ctrlLedOn(controller, 0u));
    TEST_ASSERT_TRUE(ctrlLedOn(controller, 1000u));
    TEST_ASSERT_TRUE(ctrlLedOn(controller, 1500u));
    TEST_ASSERT_TRUE(ctrlLedOn(controller, 123456u));
}

void test_led_error_fast_blink(void)
{
    ctrlAutoResult(controller, false);
    TEST_ASSERT_TRUE(ctrlLedOn(controller, 0u));
    TEST_ASSERT_TRUE(ctrlLedOn(controller, 199u));
    TEST_ASSERT_FALSE(ctrlLedOn(controller, 200u));
    TEST_ASSERT_FALSE(ctrlLedOn(controller, 399u));
    TEST_ASSERT_TRUE(ctrlLedOn(controller, 400u));
}

void test_led_error_wins_over_matlab(void)
{
    ctrlInit(controller, SWITCH_CLOSED, 0u);
    ctrlMatlabLine(controller, 0u, false);
    TEST_ASSERT_TRUE(ctrlLedOn(controller, 0u));
    TEST_ASSERT_FALSE(ctrlLedOn(controller, 200u));
}

void test_led_millis_wrap(void)
{
    TEST_ASSERT_FALSE(ctrlLedOn(controller, 4294967000u));
    TEST_ASSERT_FALSE(ctrlLedOn(controller, 4294967295u));
    TEST_ASSERT_TRUE(ctrlLedOn(controller, 0u));
    ctrlInit(controller, SWITCH_CLOSED, 0u);
    TEST_ASSERT_TRUE(ctrlLedOn(controller, 4294967295u));
}

void test_status_asked_at_once_at_start(void)
{
    ctrlInit(controller, SWITCH_OPEN, 1000u);
    TEST_ASSERT_TRUE(ctrlStatusDue(controller, 1000u));
    TEST_ASSERT_FALSE(ctrlStatusDue(controller, 1001u));
}

void test_status_retry_every_250_ms_until_known(void)
{
    ctrlInit(controller, SWITCH_OPEN, 0u);
    TEST_ASSERT_TRUE(ctrlStatusDue(controller, 0u));
    TEST_ASSERT_FALSE(ctrlStatusDue(controller, STATUS_RETRY_MS - 1u));
    TEST_ASSERT_TRUE(ctrlStatusDue(controller, STATUS_RETRY_MS));
}

void test_nothing_sent_before_first_answer(void)
{
    ctrlInit(controller, SWITCH_OPEN, 0u);
    TEST_ASSERT_TRUE(ctrlBlocked(controller));
    TEST_ASSERT_FALSE(ctrlAutoDue(controller, 0u));
    TEST_ASSERT_FALSE(controller.error);
}

void test_first_answer_aus_latches_and_auto_runs(void)
{
    ctrlInit(controller, SWITCH_OPEN, 0u);
    TEST_ASSERT_EQUAL(RESULT_LATCHED, ctrlStatus(controller, true, 0u, 250u));
    TEST_ASSERT_FALSE(controller.imuSwitchOn);
    TEST_ASSERT_FALSE(ctrlBlocked(controller));
    TEST_ASSERT_TRUE(ctrlAutoDue(controller, 250u));
}

void test_first_answer_an_latches_on(void)
{
    ctrlInit(controller, SWITCH_OPEN, 0u);
    TEST_ASSERT_EQUAL(RESULT_LATCHED, ctrlStatus(controller, true, STATUS_IMU_ON, 0u));
    TEST_ASSERT_TRUE(controller.imuSwitchOn);
}

void test_first_answer_an_while_moving_latches_on(void)
{
    ctrlInit(controller, SWITCH_OPEN, 0u);
    TEST_ASSERT_EQUAL(RESULT_LATCHED, ctrlStatus(controller, true, STATUS_IMU_ON | STATUS_MOVING, 0u));
    TEST_ASSERT_TRUE(controller.imuSwitchOn);
}

void test_no_answer_waits_then_faults_at_5_s(void)
{
    ctrlInit(controller, SWITCH_OPEN, 0u);
    TEST_ASSERT_EQUAL(RESULT_WAITING, ctrlStatus(controller, false, 0u, STARTUP_WAIT_MS - 1u));
    TEST_ASSERT_EQUAL(FAULT_NONE, controller.fault);
    TEST_ASSERT_EQUAL(RESULT_FAULT, ctrlStatus(controller, false, 0u, STARTUP_WAIT_MS));
    TEST_ASSERT_EQUAL(FAULT_NO_ANSWER, controller.fault);
    TEST_ASSERT_TRUE(ctrlBlocked(controller));
    TEST_ASSERT_FALSE(ctrlLedOn(controller, 200u));
    TEST_ASSERT_TRUE(ctrlLedOn(controller, 400u));
}

void test_after_latch_asked_every_2_s(void)
{
    ctrlInit(controller, SWITCH_OPEN, 0u);
    TEST_ASSERT_TRUE(ctrlStatusDue(controller, 0u));
    TEST_ASSERT_EQUAL(RESULT_LATCHED, ctrlStatus(controller, true, 0u, 0u));
    TEST_ASSERT_FALSE(ctrlStatusDue(controller, COMPASS_CHECK_MS - 1u));
    TEST_ASSERT_TRUE(ctrlStatusDue(controller, COMPASS_CHECK_MS));
    TEST_ASSERT_FALSE(ctrlStatusDue(controller, COMPASS_CHECK_MS + 1u));
}

void test_moving_bit_is_not_a_switch_change(void)
{
    TEST_ASSERT_EQUAL(RESULT_SAME, ctrlStatus(controller, true, STATUS_MOVING, 2000u));
    TEST_ASSERT_EQUAL(FAULT_NONE, controller.fault);
}

void test_switch_moved_faults(void)
{
    TEST_ASSERT_EQUAL(RESULT_FAULT, ctrlStatus(controller, true, STATUS_IMU_ON, 2000u));
    TEST_ASSERT_EQUAL(FAULT_SWITCH_MOVED, controller.fault);
    TEST_ASSERT_TRUE(ctrlBlocked(controller));
    TEST_ASSERT_FALSE(ctrlAutoDue(controller, 2000u));
    TEST_ASSERT_FALSE(ctrlLedOn(controller, 2200u));
    TEST_ASSERT_TRUE(ctrlLedOn(controller, 2400u));
}

void test_fault_is_permanent(void)
{
    TEST_ASSERT_EQUAL(RESULT_FAULT, ctrlStatus(controller, true, STATUS_IMU_ON, 2000u));
    ctrlAutoResult(controller, true);
    (void)flipSwitch(SWITCH_CLOSED, 3000u);
    ctrlMatlabLine(controller, 4000u, true);
    TEST_ASSERT_EQUAL(FAULT_SWITCH_MOVED, controller.fault);
    TEST_ASSERT_TRUE(ctrlBlocked(controller));
    TEST_ASSERT_FALSE(ctrlLedOn(controller, 4200u));
    TEST_ASSERT_FALSE(ctrlStatusDue(controller, 100000u));
    TEST_ASSERT_EQUAL(RESULT_IGNORED, ctrlStatus(controller, true, 0u, 100000u));
    TEST_ASSERT_EQUAL(FAULT_SWITCH_MOVED, controller.fault);
}

void test_missed_answer_after_latch_is_not_a_fault(void)
{
    TEST_ASSERT_EQUAL(RESULT_NO_ANSWER, ctrlStatus(controller, false, 0u, 2000u));
    TEST_ASSERT_EQUAL(FAULT_NONE, controller.fault);
    TEST_ASSERT_FALSE(ctrlBlocked(controller));
}

void test_init_forgets_the_latch(void)
{
    TEST_ASSERT_FALSE(ctrlBlocked(controller));
    ctrlInit(controller, SWITCH_OPEN, 0u);
    TEST_ASSERT_TRUE(ctrlBlocked(controller));
}

void test_status_timer_across_millis_wrap(void)
{
    ctrlInit(controller, SWITCH_OPEN, NEAR_WRAP_MS);
    TEST_ASSERT_TRUE(ctrlStatusDue(controller, NEAR_WRAP_MS));
    TEST_ASSERT_EQUAL(RESULT_LATCHED, ctrlStatus(controller, true, 0u, NEAR_WRAP_MS));
    TEST_ASSERT_FALSE(ctrlStatusDue(controller, NEAR_WRAP_MS + COMPASS_CHECK_MS - 1u));
    TEST_ASSERT_TRUE(ctrlStatusDue(controller, NEAR_WRAP_MS + COMPASS_CHECK_MS));
}

int runAllTests(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_init_takes_mode_from_switch);
    RUN_TEST(test_same_position_never_changes_mode);
    RUN_TEST(test_debounce_needs_50_ms_still);
    RUN_TEST(test_bounce_restarts_debounce);
    RUN_TEST(test_mode_change_reported_once);
    RUN_TEST(test_mode_change_clears_error);
    RUN_TEST(test_auto_first_cycle_runs_at_once);
    RUN_TEST(test_auto_every_30_s);
    RUN_TEST(test_auto_never_due_in_matlab);
    RUN_TEST(test_back_to_auto_runs_at_once);
    RUN_TEST(test_auto_cycle_across_millis_wrap);
    RUN_TEST(test_auto_result_sets_and_clears_error);
    RUN_TEST(test_matlab_timeout_after_180_s);
    RUN_TEST(test_matlab_timeout_reported_once);
    RUN_TEST(test_no_timeout_in_auto);
    RUN_TEST(test_good_line_restarts_timeout_and_clears_error);
    RUN_TEST(test_line_not_answered_sets_error);
    RUN_TEST(test_switch_to_matlab_restarts_timeout);
    RUN_TEST(test_matlab_timeout_across_millis_wrap);
    RUN_TEST(test_led_auto_slow_blink);
    RUN_TEST(test_led_matlab_always_on);
    RUN_TEST(test_led_error_fast_blink);
    RUN_TEST(test_led_error_wins_over_matlab);
    RUN_TEST(test_led_millis_wrap);
    RUN_TEST(test_status_asked_at_once_at_start);
    RUN_TEST(test_status_retry_every_250_ms_until_known);
    RUN_TEST(test_nothing_sent_before_first_answer);
    RUN_TEST(test_first_answer_aus_latches_and_auto_runs);
    RUN_TEST(test_first_answer_an_latches_on);
    RUN_TEST(test_first_answer_an_while_moving_latches_on);
    RUN_TEST(test_no_answer_waits_then_faults_at_5_s);
    RUN_TEST(test_after_latch_asked_every_2_s);
    RUN_TEST(test_moving_bit_is_not_a_switch_change);
    RUN_TEST(test_switch_moved_faults);
    RUN_TEST(test_fault_is_permanent);
    RUN_TEST(test_missed_answer_after_latch_is_not_a_fault);
    RUN_TEST(test_init_forgets_the_latch);
    RUN_TEST(test_status_timer_across_millis_wrap);
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