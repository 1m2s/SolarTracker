#include "Controller.h"
#include "TimeChecks.h"
#include "config/Board.h"

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
    controller.imuKnown        = false;
    controller.imuSwitchOn     = false;
    controller.fault           = FAULT_NONE;
    controller.startMs         = nowMs;
    controller.lastStatusMs    = nowMs;
    controller.askNow          = true;
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
    if (ctrlBlocked(controller)) {
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
    if (controller.error || (controller.fault != FAULT_NONE)) {
        return (nowMs / ERROR_BLINK_MS) % 2u == 0u;
    }
    if (controller.mode == MODE_MATLAB) {
        return true;
    }
    return (nowMs / AUTO_BLINK_MS) % 2u == 0u;
}

bool ctrlStatusDue(Controller &controller, uint32_t nowMs)
{
    if (controller.fault != FAULT_NONE) {
        return false;
    }
    uint32_t intervalMs = controller.imuKnown ? COMPASS_CHECK_MS : STATUS_RETRY_MS;
    if (!controller.askNow && !isDue(nowMs, controller.lastStatusMs, intervalMs)) {
        return false;
    }
    controller.askNow       = false;
    controller.lastStatusMs = nowMs;
    return true;
}

StatusResult ctrlStatus(Controller &controller, bool answered, uint8_t status, uint32_t nowMs)
{
    if (controller.fault != FAULT_NONE) {
        return RESULT_IGNORED;
    }

    bool switchOn = (status & STATUS_IMU_ON) != 0u;

    if (!controller.imuKnown) {
        if (answered) {
            controller.imuKnown    = true;
            controller.imuSwitchOn = switchOn;
            return RESULT_LATCHED;
        }
        if (isDue(nowMs, controller.startMs, STARTUP_WAIT_MS)) {
            controller.fault = FAULT_NO_ANSWER;
            return RESULT_FAULT;
        }
        return RESULT_WAITING;
    }

    if (!answered) {
        return RESULT_NO_ANSWER;
    }
    if (switchOn != controller.imuSwitchOn) {
        controller.fault = FAULT_SWITCH_MOVED;
        return RESULT_FAULT;
    }
    return RESULT_SAME;
}

bool ctrlBlocked(const Controller &controller)
{
    return !controller.imuKnown || (controller.fault != FAULT_NONE);
}