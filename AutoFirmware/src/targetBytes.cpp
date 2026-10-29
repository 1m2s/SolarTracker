#include "targetBytes.h"
#include "toUint8.h"

// Park position = start position of the right Nano
const uint8_t PARK_A8 = 90;      // 90 * 2 = 180 deg = south
const uint8_t PARK_G8 = 0;       // 0 deg = panel vertical

// Safe azimuth window. The sun is always inside it during the day
// (year scan: 53.7 .. 306.3 deg), and it never crosses north (0/360).
const float AZ_MIN = 50.0f;
const float AZ_MAX = 310.0f;

bool targetBytes(float alpha, float gamma, uint8_t &a8, uint8_t &g8)
{
    bool night  = !(gamma > 0.0f);                          // sun below horizon (or NaN)
    bool unsafe = !(alpha >= AZ_MIN && alpha <= AZ_MAX);    // outside window (or NaN)

    if (night || unsafe) {
        a8 = PARK_A8;
        g8 = PARK_G8;
        return true;                 // parked
    }

    a8 = toUint8(alpha / 2.0f);      // day: same as MATLAB
    g8 = toUint8(gamma);
    return false;                    // following the sun
}

/* Notes
1) Why park at night?
   At midnight the azimuth jumps from about 346 to 2 deg.
   The right Nano would turn about 344 deg backwards.
   That can damage the cables. Parking avoids it.

2) Why park at south?
   South (180 deg) and vertical (0 deg) is where the right Nano
   thinks it is after power-on. So after a power cut at night,
   the real position and the assumed position still match.

3) Night rule: gamma > 0 means the sun is above the horizon.
   !(gamma > 0) is also true for NaN, like in toUint8.

4) Safe window: a second guard. In normal use it never fires,
   because the sun stays between 53.7 and 306.3 deg all year.
   It only fires if something is broken (e.g. alpha is NaN).
   Then we park instead of sending a strange direction.

5) The & in a8 and g8: the function fills both bytes,
   like SonnenStand fills alpha and gamma.
*/