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
static const uint32_t FROZEN_MS = 2000;

/******************************************************************************
 Function Definitions
******************************************************************************/
bool isDue(uint32_t now, uint32_t last, uint32_t interval)
{
    /* A6: wrap-safe */
    uint32_t passed = now - last;
    return passed >= interval;
}

bool clockFrozen(ClockWatch &w, uint32_t nowMs, uint32_t rtcSecs)
{
    /*
     * w.ms is set only when the time changes, so many quick readings of the
     * same second cannot keep pushing the 2 s limit back.
     */
    if (!w.have || rtcSecs != w.secs) {
        w.have = true;
        w.secs = rtcSecs;
        w.ms   = nowMs;
        return false;
    }
    return isDue(nowMs, w.ms, FROZEN_MS);
}