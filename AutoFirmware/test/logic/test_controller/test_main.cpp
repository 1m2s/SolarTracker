/**
 * @file test_main.cpp
 * @brief Host tests for Controller: debounce, AUTO timing, MATLAB timeout, LED
 *
 * Every test drives the Controller only through its ctrl functions, with
 * made-up millis() values, the way main.cpp does with the real ones.
 *
 * @author Mohamed Sellami
 * @date 2026-11-03
 *
 */
#include <unity.h>
#include "Controller.h"

/******************************************************************************
 Constants
******************************************************************************/
/* Switch readings, so the tests read like the hardware. */
static const bool SWITCH_OPEN   = true;
static const bool SWITCH_CLOSED = false;

/* millis() a few seconds before the 32-bit wrap. */
static const uint32_t NEAR_WRAP_MS = 4294967000u;

/******************************************************************************
 Private Data
******************************************************************************/
/* Fresh for every test: setUp() runs before each one. */
static Controller controller;

/******************************************************************************
 Private Function Declarations
******************************************************************************/
/**
 * @brief Flip the switch at startMs and hold it still past the debounce time
 *
 * @param[in] switchOpen  New switch reading, true = open = AUTO.
 * @param[in] startMs     millis() when the switch is flipped.
 *
 * @return millis() when the new mode was accepted (startMs + DEBOUNCE_MS).
 */
static uint32_t flipSwitch(bool switchOpen, uint32_t startMs);

/******************************************************************************
 Function Definitions
******************************************************************************/
void setUp(void)
{
    ctrlInit(controller, SWITCH_OPEN, 0u);
}

void tearDown(void) {}

static uint32_t flipSwitch(bool switchOpen, uint32_t startMs)
{
    uint32_t acceptMs = startMs + DEBOUNCE_MS;
    TEST_ASSERT_FALSE(ctrlSwitch(controller, switchOpen, startMs));
    TEST_ASSERT_TRUE(ctrlSwitch(controller, switchOpen, acceptMs));
    return acceptMs;
}

/* ---------- start ---------- */

void test_init_takes_mode_from_switch(void)
{
    ctrlInit(controller, SWITCH_OPEN, 0u);
    TEST_ASSERT_EQUAL(MODE_AUTO, controller.mode);
    ctrlInit(controller, SWITCH_CLOSED, 0u);
    TEST_ASSERT_EQUAL(MODE_MATLAB, controller.mode);
    TEST_ASSERT_FALSE(controller.error);
}

/* ---------- debounce ---------- */

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
    /* 50 ms after the first flip, but only 40 ms after the last bounce. */
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

/* ---------- AUTO timing ---------- */

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
    /* Only about 3 s after the last cycle, but switching to AUTO runs one now. */
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

/* ---------- MATLAB timeout ---------- */

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
    /* The line itself was good, so the timeout still restarts. */
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

/* ---------- LED (moved from test_ledPattern) ---------- */

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

/*
 * 4294967295 / 1000 = 4294967 (odd), so the LED is off just before the wrap
 * and on at 0. The last off phase is only 296 ms long. Harmless.
 */
void test_led_millis_wrap(void)
{
    TEST_ASSERT_FALSE(ctrlLedOn(controller, 4294967000u));
    TEST_ASSERT_FALSE(ctrlLedOn(controller, 4294967295u));
    TEST_ASSERT_TRUE(ctrlLedOn(controller, 0u));
    ctrlInit(controller, SWITCH_CLOSED, 0u);
    TEST_ASSERT_TRUE(ctrlLedOn(controller, 4294967295u));
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
    return UNITY_END();
}

#ifdef ARDUINO
#include <Arduino.h>
void setup()
{
    /* Wait for the Serial monitor to open. */
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