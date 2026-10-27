#include <math.h>
#include "Deklination.h"
#include "grad2rad.h"

// From MATLAB: delta = 0.3948-23.2559*cos(a1)-0.391*cos(a2)-0.1764*cos(a3);
float Deklination(int d)
{
    // Abweichung zwischen Magnetfeld und Geographie (original comment, see Notes)
    float Js    = 360.0f * d / 365.0f;     // Umrechnung des Tages in Laufvariable (in Grad)
    float a1    = grad2rad(Js + 9.1f);
    float a2    = grad2rad(2.0f * Js + 5.4f);
    float a3    = grad2rad(3.0f * Js + 26.0f);
    float delta = 0.3948f - 23.2559f * cosf(a1) - 0.391f * cosf(a2) - 0.1764f * cosf(a3); // delta in Grad
    return delta;
}

/* Notes
 * - delta is the angle of the sun above/below the equator.
 *   About +23.4 in June, 0 in March/Sept, -23.4 in December.
 * - The original comment about "Magnetfeld" is wrong.
 *   This is the sun's declination, not magnetic.
 * - The original says Js is in rad. It is in degrees.
 * - 360.0f first, so d is turned into a float. No integer division.
 */