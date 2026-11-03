#include "ClockWatch.h"
#include "isDue.h"

const uint32_t FROZEN_MS = 2000;   // a running RTC ticks at least once in this time

bool clockFrozen(ClockWatch &w, uint32_t nowMs, uint32_t rtcSecs)
{
    if (!w.have || rtcSecs != w.secs) {   // first reading, or the clock moved
        w.have = true;
        w.secs = rtcSecs;
        w.ms   = nowMs;                   // start timing this new value
        return false;
    }
    return isDue(nowMs, w.ms, FROZEN_MS); // same time for too long -> frozen
}

/* Notes
1) Why 2 s and not 1 s?
   The RTC second changes once per second. Two readings 1 s apart
   can still show the same second, because the Nano's millis()
   is not exactly the RTC's second. 2 s always has a tick in it.

2) w.ms is only set when the time CHANGES.
   So many quick readings of the same second (switch flipped
   several times) cannot keep pushing the 2 s back.

3) A frozen clock stays frozen on every call, until the time moves.
*/