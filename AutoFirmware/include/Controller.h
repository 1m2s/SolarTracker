#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <stdint.h>
#include "config/Settings.h"

enum Mode : uint8_t {
    MODE_AUTO   = 0,
    MODE_MATLAB = 1
};

enum ImuFault : uint8_t {
    FAULT_NONE         = 0,
    FAULT_NO_ANSWER    = 1,
    FAULT_SWITCH_MOVED = 2
};

enum StatusResult : uint8_t {
    RESULT_WAITING   = 0,
    RESULT_LATCHED   = 1,
    RESULT_SAME      = 2,
    RESULT_NO_ANSWER = 3,
    RESULT_FAULT     = 4,
    RESULT_IGNORED   = 5
};

enum CompassResult : uint8_t {
    COMPASS_OK           = 0,
    COMPASS_BAD_IGNORED  = 1,
    COMPASS_FAILED       = 2,
    COMPASS_STILL_FAILED = 3,
    COMPASS_RECOVERED    = 4
};

struct Controller {
    Mode     mode;
    bool     error;
    bool     runNow;
    uint32_t lastCycleMs;
    bool     lastSwitchOpen;
    uint32_t switchChangedMs;
    uint32_t lastMatlabMs;
    bool     timedOut;
    bool     imuKnown;
    bool     imuSwitchOn;
    ImuFault fault;
    uint32_t startMs;
    uint32_t lastStatusMs;
    bool     askNow;
    uint8_t  badReads;
    uint8_t  goodReads;
    bool     compassFailed;
    bool     hasHeading;
    float    headingDeg;
};

void ctrlInit(Controller &controller, bool switchOpen, uint32_t nowMs);

bool ctrlSwitch(Controller &controller, bool switchOpen, uint32_t nowMs);

bool ctrlAutoDue(Controller &controller, uint32_t nowMs);

void ctrlAutoResult(Controller &controller, bool ok);

void ctrlMatlabLine(Controller &controller, uint32_t nowMs, bool sendOk);

bool ctrlMatlabTimedOut(Controller &controller, uint32_t nowMs);

bool ctrlLedOn(const Controller &controller, uint32_t nowMs);

bool ctrlStatusDue(Controller &controller, uint32_t nowMs);

StatusResult ctrlStatus(Controller &controller, bool answered, uint8_t status, uint32_t nowMs);

bool ctrlBlocked(const Controller &controller);

CompassResult ctrlCompassRead(Controller &controller, bool readOk, float headingDeg);

bool ctrlCompassUsable(const Controller &controller);

#endif