/**
 * @file TimeChecks.h
 * @brief Time checks: "is it time yet?" and "is the RTC frozen?"
 *
 * isDue is the timer for everything that runs every X ms (AUTO cycle,
 * debounce, MATLAB timeout). clockFrozen spots an RTC that shows a valid time
 * but no longer moves (e.g. a dead crystal). Both work with millis() and stay
 * correct when millis() wraps after about 49.7 days.
 *
 * Target: Arduino Nano (ATmega328P) and PC (host tests). Level: L2.
 *
 * @author Mohamed Sellami
 * @date 2026-11-03
 *
 */
#ifndef TIMECHECKS_H
#define TIMECHECKS_H

#include <stdint.h>

/******************************************************************************
 Types
******************************************************************************/
/**
 * @brief Last RTC reading, to see if the clock still moves
 *
 * Start with: ClockWatch w = {false, 0, 0};
 */
struct ClockWatch {
    bool     have;    /**< true = a reading is stored */
    uint32_t secs;    /**< That RTC time, s since 2000-01-01 (RtcDateTime::TotalSeconds) */
    uint32_t ms;      /**< millis() when this RTC time was first seen */
};

/******************************************************************************
 Function Declarations
******************************************************************************/
/**
 * @brief Check if at least interval ms have passed since last
 *
 * @param[in] now       millis() now.
 * @param[in] last      millis() at the start of the wait.
 * @param[in] interval  Wait time, ms, 0..4294967295.
 *
 * @return true if now - last >= interval, false otherwise.
 *
 * @note Correct across the millis() wrap (A6), because unsigned subtraction
 *       wraps the same way. The check now >= last + interval would not be.
 */
bool isDue(uint32_t now, uint32_t last, uint32_t interval);

/**
 * @brief Check if the RTC time has stopped moving
 *
 * Call once per AUTO cycle with the RTC time just read. Stores the reading
 * in w whenever it is new.
 *
 * @param[in,out] w        Last reading. Updated when the RTC time changed.
 * @param[in]     nowMs    millis() now.
 * @param[in]     rtcSecs  RTC time just read, s since 2000-01-01.
 *
 * @return true if the RTC time has not changed for at least 2000 ms,
 *         false on the first call and whenever the time moved.
 */
bool clockFrozen(ClockWatch &w, uint32_t nowMs, uint32_t rtcSecs);

#endif /* TIMECHECKS_H */