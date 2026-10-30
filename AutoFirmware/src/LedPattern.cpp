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

/* Notes
1) How the blink works:
   ms / 1000 counts the seconds: 0, 1, 2, 3 ...
   % 2 asks: even or odd?
   Even second -> on. Odd second -> off.
   Same idea for 200 ms, just faster.
 
2) Order of the ifs = priority.
   An error always wins. You see it in AUTO and in MATLAB.
 
3) No Arduino.h here. So the PC can test it too.
   main.cpp gives millis() as ms.
 
4) millis() wraps to 0 after about 49.7 days.
   Then one blink is a bit short. Only the LED. Harmless.
*/