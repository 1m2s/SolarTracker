#include "MEZ.h"

// From MATLAB: Min_MEZ = Minute + (Stunde - tf)*60, tf = 1 in summer time
int MEZ(int Stunde, int Minute, int tf)
{
    // Ist Sommerzeit?
    int MEZ_Stunde = Stunde;
    if (tf == 1) {
        MEZ_Stunde = Stunde - 1;
    } else if (tf == 0) {
        MEZ_Stunde = Stunde;
    }

    int MEZ_Minute = Minute;
    int Min_MEZ    = MEZ_Minute + MEZ_Stunde * 60;
    return Min_MEZ;
}

/* Notes
 * - MEZ_Stunde starts as Stunde, so it always has a value.
 * - At 00:30 in summer time the result is -30, not 23:30 of the
 *   day before. Same as MATLAB. Will be fixed later.
 * - tf = 1 means summer time. Later the Arduino must find this
 *   out by itself (needs a clock module).
 */