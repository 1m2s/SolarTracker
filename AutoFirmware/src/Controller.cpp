#include "Controller.h"
#include "TimeChecks.h"
#include "config/Board.h"

static const uint32_t ERROR_BLINK_MS = 200uL;
static const uint32_t AUTO_BLINK_MS  = 1000uL;
static const float    START_AZIMUTH_DEG = 180.0f;
static const uint32_t TURN_START_MS  = 500uL;

static bool startupBusy(const Controller &controller)
{
    return (controller.startup != STARTUP_WAITING) && (controller.startup != STARTUP_DONE);
}

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
    controller.badReads        = 0u;
    controller.goodReads       = 0u;
    controller.compassFailed   = false;
    controller.hasHeading      = false;
    controller.headingDeg      = 0.0f;
    controller.lastSentDeg     = START_AZIMUTH_DEG;
    controller.sunAzimuthDeg   = 0.0f;
    controller.sunElevationDeg = 0.0f;
    controller.sunValid        = false;
    controller.startup         = STARTUP_WAITING;
    controller.startupMs       = nowMs;
    controller.panelDeg        = 0.0f;
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

    controller.mode     = newMode;
    controller.error    = false;
    controller.sunValid = false;
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
    if (controller.error || (controller.fault != FAULT_NONE) || controller.compassFailed) {
        return (nowMs / ERROR_BLINK_MS) % 2u == 0u;
    }
    if (startupBusy(controller)) {
        return false;
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
    bool fastAsk = !controller.imuKnown || (controller.startup == STARTUP_TURNING);
    uint32_t intervalMs = fastAsk ? STATUS_RETRY_MS : COMPASS_CHECK_MS;
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
    return !controller.imuKnown || (controller.fault != FAULT_NONE) || startupBusy(controller);
}

CompassResult ctrlCompassRead(Controller &controller, bool readOk, float headingDeg)
{
    bool goodRead = readOk && (headingDeg >= 0.0f) && (headingDeg < 360.0f);

    if (goodRead) {
        controller.badReads   = 0u;
        controller.hasHeading = true;
        controller.headingDeg = headingDeg;
        if (controller.goodReads < COMPASS_RECOVER_READS) {
            controller.goodReads++;
        }
        if (!controller.compassFailed) {
            return COMPASS_OK;
        }
        if (controller.goodReads >= COMPASS_RECOVER_READS) {
            controller.compassFailed = false;
            return COMPASS_RECOVERED;
        }
        return COMPASS_STILL_FAILED;
    }

    controller.goodReads = 0u;
    if (controller.badReads < COMPASS_FAIL_READS) {
        controller.badReads++;
    }
    if (controller.compassFailed) {
        return COMPASS_STILL_FAILED;
    }
    if (controller.badReads >= COMPASS_FAIL_READS) {
        controller.compassFailed = true;
        return COMPASS_FAILED;
    }
    return COMPASS_BAD_IGNORED;
}

bool ctrlCompassUsable(const Controller &controller)
{
    return controller.hasHeading && !controller.compassFailed;
}

void ctrlSunAngles(Controller &controller, float azimuthDeg, float elevationDeg, bool parked)
{
    controller.sunAzimuthDeg   = azimuthDeg;
    controller.sunElevationDeg = elevationDeg;
    controller.sunValid        = !parked;
}

void ctrlSendDone(Controller &controller, uint32_t nowMs, float sentAzimuthDeg, bool ackOk)
{
    controller.lastStatusMs = nowMs;
    if (ackOk) {
        controller.lastSentDeg = sentAzimuthDeg;
    }
}

bool ctrlReadCompass(const Controller &controller, uint8_t status)
{
    bool motorStill = (status & STATUS_MOVING) == 0u;
    return controller.imuKnown && controller.imuSwitchOn
        && (controller.fault == FAULT_NONE) && motorStill;
}

bool ctrlResendAllowed(const Controller &controller)
{
    return controller.imuKnown && controller.imuSwitchOn
        && (controller.fault == FAULT_NONE) && controller.sunValid
        && ctrlCompassUsable(controller) && !controller.error;
}

StartupStep ctrlStartupStep(Controller &controller, uint32_t nowMs)
{
    if (controller.fault != FAULT_NONE) {
        return STEP_NONE;
    }

    switch (controller.startup) {
    case STARTUP_WAITING:
        if (!controller.imuKnown) {
            return STEP_NONE;
        }
        if (!controller.imuSwitchOn) {
            controller.startup = STARTUP_DONE;
            return STEP_DONE;
        }
        controller.startup   = STARTUP_SETTLING;
        controller.startupMs = nowMs;
        return STEP_SETTLE;

    case STARTUP_SETTLING:
        if (!isDue(nowMs, controller.startupMs, ALIGN_SETTLE_MS)) {
            return STEP_NONE;
        }
        controller.startup   = STARTUP_MEASURING;
        controller.startupMs = nowMs;
        return STEP_START_MEASURE;

    case STARTUP_MEASURING:
        if (!isDue(nowMs, controller.startupMs, ALIGN_MEASURE_MS)) {
            return STEP_READ;
        }
        controller.startup = STARTUP_AIMING;
        return STEP_AIM;

    case STARTUP_HOLDING:
        if (!isDue(nowMs, controller.startupMs, ALIGN_HOLD_MS)) {
            return STEP_NONE;
        }
        controller.startup = STARTUP_DONE;
        controller.runNow  = true;
        return STEP_DONE;

    default:
        return STEP_NONE;
    }
}

ImuFault ctrlStartupMeasured(Controller &controller, bool averageOk, bool southOk, float panelDeg, uint32_t nowMs)
{
    if ((controller.startup != STARTUP_AIMING) || (controller.fault != FAULT_NONE)) {
        return controller.fault;
    }
    if (!averageOk) {
        controller.fault = FAULT_COMPASS;
        return controller.fault;
    }
    if (!southOk) {
        controller.fault = FAULT_NOT_SOUTH;
        return controller.fault;
    }
    controller.panelDeg  = panelDeg;
    controller.startup   = STARTUP_TURNING;
    controller.startupMs = nowMs;
    return FAULT_NONE;
}

bool ctrlStartupMotor(Controller &controller, uint8_t status, uint32_t nowMs)
{
    if ((controller.startup != STARTUP_TURNING) || (controller.fault != FAULT_NONE)) {
        return false;
    }
    if (!isDue(nowMs, controller.startupMs, TURN_START_MS)) {
        return false;
    }
    if ((status & STATUS_MOVING) != 0u) {
        return false;
    }
    controller.startup   = STARTUP_HOLDING;
    controller.startupMs = nowMs;
    return true;
}