#ifndef CLOCKWATCH_H
#define CLOCKWATCH_H

#include <stdint.h>

// Remembers one RTC reading, to see if the clock still moves.
struct ClockWatch {
    bool     have;    // a reading is stored
    uint32_t secs;    // that RTC time (seconds since 2000)
    uint32_t ms;      // millis() when this time was first seen
};

// Call once per AUTO cycle with the RTC time just read.
// True when the RTC time has not changed for at least 2 s of millis().
// Start with: ClockWatch w = {false, 0, 0};
bool clockFrozen(ClockWatch &w, uint32_t nowMs, uint32_t rtcSecs);

#endif