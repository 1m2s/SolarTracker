#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <stdint.h>
#include "config/Settings.h"

enum Mode : uint8_t {
    MODE_AUTO   = 0,
    MODE_MATLAB = 1
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
};

void ctrlInit(Controller &controller, bool switchOpen, uint32_t nowMs);

bool ctrlSwitch(Controller &controller, bool switchOpen, uint32_t nowMs);

bool ctrlAutoDue(Controller &controller, uint32_t nowMs);

void ctrlAutoResult(Controller &controller, bool ok);

void ctrlMatlabLine(Controller &controller, uint32_t nowMs, bool sendOk);

bool ctrlMatlabTimedOut(Controller &controller, uint32_t nowMs);

bool ctrlLedOn(const Controller &controller, uint32_t nowMs);

#endif