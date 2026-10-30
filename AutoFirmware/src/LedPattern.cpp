#include "LedPattern.h"

const uint32_t FAST_MS = 200;     // error:   0.2 s on, 0.2 s off
const uint32_t SLOW_MS = 1000;    // AUTO OK: 1 s on, 1 s off

bool ledOn(Mode mode, bool error, uint32_t ms)
{
    if (error) {
        return (ms / FAST_MS) % 2 == 0;    // priority 1: fast blink
    }
    if (mode == MODE_MATLAB) {
        return true;                       // priority 2: always on
    }
    return (ms / SLOW_MS) % 2 == 0;        // priority 3: slow blink
}