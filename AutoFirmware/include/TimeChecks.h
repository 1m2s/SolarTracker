#ifndef TIMECHECKS_H
#define TIMECHECKS_H

#include <stdint.h>

struct ClockWatch {
    bool     hasReading;
    uint32_t rtcSeconds;
    uint32_t firstSeenMs;
};

bool isDue(uint32_t nowMs, uint32_t lastMs, uint32_t intervalMs);

bool clockFrozen(ClockWatch &watch, uint32_t nowMs, uint32_t rtcSeconds);

#endif