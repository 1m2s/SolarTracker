#include "Controller.h"
#include "TimeChecks.h"

static const uint32_t ERROR_BLINK_MS = 200uL;
static const uint32_t AUTO_BLINK_MS  = 1000uL;

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
    if (controller.error) {
        return (nowMs / ERROR_BLINK_MS) % 2u == 0u;
    }
    if (controller.mode == MODE_MATLAB) {
        return true;
    }
    return (nowMs / AUTO_BLINK_MS) % 2u == 0u;
}