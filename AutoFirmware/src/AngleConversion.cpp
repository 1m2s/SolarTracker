#include <math.h>
#include "AngleConversion.h"

float degToRad(float angleDeg)
{
    return angleDeg * (float)M_PI / 180.0f;
}

float radToDeg(float angleRad)
{
    return angleRad / (float)M_PI * 180.0f;
}