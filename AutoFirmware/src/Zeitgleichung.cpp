#include <math.h>
#include "Zeitgleichung.h"
#include "grad2rad.h"

// From MATLAB: zgl = 0.0066+7.3525*cos(a1)+9.9359*cos(a2)+0.3387*cos(a3);
float Zeitgleichung(float Jg)
{
    float a1 = grad2rad(Jg + 85.9f);
    float a2 = grad2rad(2.0f * Jg + 108.9f);
    float a3 = grad2rad(3.0f * Jg + 105.2f);
    float zgl = 0.0066f + 7.3525f * cosf(a1) + 9.9359f * cosf(a2) + 0.3387f * cosf(a3);
    return zgl;
}

/* Notes
 * - Result is in minutes: WOZ = MOZ + zgl.
 * - Jg is the day of the year as an angle in degrees.
 * - cosf needs radians, so grad2rad first.
 * - All numbers end in f so everything stays float.
 */