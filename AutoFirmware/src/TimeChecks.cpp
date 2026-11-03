#include "TimeChecks.h"

static const uint32_t FROZEN_MS = 2000uL;

bool isDue(uint32_t nowMs, uint32_t lastMs, uint32_t intervalMs)
{
    uint32_t passedMs = nowMs - lastMs;
    return passedMs >= intervalMs;
}

bool clockFrozen(ClockWatch &watch, uint32_t nowMs, uint32_t rtcSeconds)
{
    if (!watch.hasReading || rtcSeconds != watch.rtcSeconds) {
        watch.hasReading  = true;
        watch.rtcSeconds  = rtcSeconds;
        watch.firstSeenMs = nowMs;
        return false;
    }
    return isDue(nowMs, watch.firstSeenMs, FROZEN_MS);
}