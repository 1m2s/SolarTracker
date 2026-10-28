#include "toUint8.h"
#include <math.h>

// From MATLAB: y = uint8(x)
uint8_t toUint8(float x)
{
    if (!(x > 0.0f))   return 0;      // negative, zero or NaN
    if (x >= 255.0f)   return 255;    // too big
    return (uint8_t)roundf(x);        // 0 < x < 255: round
}

/* Notes
1) Why this function?
   The slave reads one byte per value (0..255).
   MATLAB makes that byte with uint8().
   C++ has no safe uint8() like that.

2) Why check first, then round?
   In C++, putting a negative float into uint8_t is
   "undefined behaviour". It can give any number.
   So we catch the bad cases first.

3) Why !(x > 0.0f) and not x <= 0.0f?
   NaN means "not a number". Every compare with NaN is false.
   x <= 0 would be false for NaN, so NaN would slip through.
   !(x > 0) is true for NaN, so we return 0.
   MATLAB: uint8(NaN) = 0. Same here.


5) Night: the elevation is negative -> 0.
   That is also what MATLAB sends at night.
*/