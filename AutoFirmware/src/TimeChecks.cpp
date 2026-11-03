/**
 * @file TimeChecks.cpp
 * @brief Implementation of the wrap-safe timer and the frozen-RTC check
 *
 * Both checks use unsigned millis() differences, so they need no special
 * case for the wrap.
 *
 * @author Mohamed Sellami
 * @date 2026-11-03
 *
 */
#include "TimeChecks.h"

/******************************************************************************
 Constants
******************************************************************************/
/*
 * A running RTC ticks at least once in this time. 1 s is not enough: millis()
 * and the RTC second are not in step, so two readings 1 s apart can still
 * show the same second.
 */
static const uint32_t FROZEN_MS = 2000uL;

/******************************************************************************
 Function Definitions
******************************************************************************/
bool isDue(uint32_t nowMs, uint32_t lastMs, uint32_t intervalMs)
{
    /* A6: wrap-safe */
    uint32_t passedMs = nowMs - lastMs;
    return passedMs >= intervalMs;
}

bool clockFrozen(ClockWatch &watch, uint32_t nowMs, uint32_t rtcSeconds)
{
    /*
     * watch.firstSeenMs is set only when the time changes, so many quick
     * readings of the same second cannot keep pushing the 2 s limit back.
     */
    if (!watch.hasReading || rtcSeconds != watch.rtcSeconds) {
        watch.hasReading  = true;
        watch.rtcSeconds  = rtcSeconds;
        watch.firstSeenMs = nowMs;
        return false;
    }
    return isDue(nowMs, watch.firstSeenMs, FROZEN_MS);
}