/**
 * @file Controller.cpp
 * @brief Implementation of the mode, timing, error and LED decisions
 *
 * Each function does what the matching lines of main.cpp did before, in the
 * same order. All time checks go through isDue, so they stay correct across
 * the millis() wrap.
 *
 * @author Mohamed Sellami
 * @date 2026-11-03
 *
 */
#include "Controller.h"
#include "TimeChecks.h"

/******************************************************************************
 Constants
******************************************************************************/
/* LED blink half-periods: error = 0.2 s on, 0.2 s off; AUTO OK = 1 s on, 1 s off. */
static const uint32_t ERROR_BLINK_MS = 200uL;
static const uint32_t AUTO_BLINK_MS  = 1000uL;

/******************************************************************************
 Function Definitions
******************************************************************************/
void ctrlInit(Controller &controller, bool switchOpen, uint32_t nowMs)
{
    controller.mode            = switchOpen ? MODE_AUTO : MODE_MATLAB;
    controller.error           = false;
    controller.runNow          = true;
    controller.lastCycleMs     = 0uL;
    controller.lastSwitchOpen  = switchOpen;
    controller.switchChangedMs = nowMs;
    controller.lastMatlabMs    = nowMs;
    controller.timedOut        = false;
}

bool ctrlSwitch(Controller &controller, bool switchOpen, uint32_t nowMs)
{
    /* Any change restarts the debounce time, so a bouncing contact never counts. */
    if (switchOpen != controller.lastSwitchOpen) {
        controller.lastSwitchOpen  = switchOpen;
        controller.switchChangedMs = nowMs;
        return false;
    }

    Mode newMode = switchOpen ? MODE_AUTO : MODE_MATLAB;
    if (newMode == controller.mode || !isDue(nowMs, controller.switchChangedMs, DEBOUNCE_MS)) {
        return false;
    }

    controller.mode  = newMode;
    controller.error = false;
    if (newMode == MODE_AUTO) {
        controller.runNow = true;
    } else {
        controller.lastMatlabMs = nowMs;
        controller.timedOut     = false;
    }
    return true;
}

bool ctrlAutoDue(Controller &controller, uint32_t nowMs)
{
    if (controller.mode != MODE_AUTO) {
        return false;
    }
    if (!controller.runNow && !isDue(nowMs, controller.lastCycleMs, AUTO_INTERVAL_MS)) {
        return false;
    }
    controller.runNow      = false;
    controller.lastCycleMs = nowMs;
    return true;
}

void ctrlAutoResult(Controller &controller, bool ok)
{
    controller.error = !ok;
}

void ctrlMatlabLine(Controller &controller, uint32_t nowMs, bool sendOk)
{
    controller.error        = !sendOk;
    controller.lastMatlabMs = nowMs;
    controller.timedOut     = false;
}

bool ctrlMatlabTimedOut(Controller &controller, uint32_t nowMs)
{
    if (controller.mode != MODE_MATLAB || controller.timedOut) {
        return false;
    }
    if (!isDue(nowMs, controller.lastMatlabMs, MATLAB_TIMEOUT_MS)) {
        return false;
    }
    controller.timedOut = true;
    controller.error    = true;
    return true;
}

bool ctrlLedOn(const Controller &controller, uint32_t nowMs)
{
    /* The order of the checks is the priority: an error always wins. */
    if (controller.error) {
        return (nowMs / ERROR_BLINK_MS) % 2u == 0u;
    }
    if (controller.mode == MODE_MATLAB) {
        return true;
    }
    return (nowMs / AUTO_BLINK_MS) % 2u == 0u;
}