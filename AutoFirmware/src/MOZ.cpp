#include "MOZ.h"

// From MATLAB: Min_MOZ = Min_MEZ-4*(15-lambda);
float MOZ(float Min_MEZ, float lambda)
{
    // lambda = Laengengrad
    float Min_MOZ = Min_MEZ - 4.0f * (15.0f - lambda);  // Minuten seit 0:00 Uhr
    return Min_MOZ;
}

/* Notes
 * - MEZ is the time at 15 deg East. We are at 9.66 deg East,
 *   so local time is behind. 4 minutes per degree.
 * - Uses float because lambda has decimals.
 * - 4.0f and 15.0f keep everything in float.
 */