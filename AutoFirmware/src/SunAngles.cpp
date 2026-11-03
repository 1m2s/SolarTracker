#include <math.h>
#include "SunAngles.h"
#include "SolarTime.h"
#include "AngleConversion.h"

static const float AZIMUTH_DIVISOR_MIN = 1.0e-3f;

float declination(int dayNumber)
{
    float yearAngleDeg   = 360.0f * (float)dayNumber / 365.0f;
    float term1Rad       = degToRad(yearAngleDeg + 9.1f);
    float term2Rad       = degToRad(2.0f * yearAngleDeg + 5.4f);
    float term3Rad       = degToRad(3.0f * yearAngleDeg + 26.0f);
    float declinationDeg = 0.3948f
                         - 23.2559f * cosf(term1Rad)
                         - 0.391f   * cosf(term2Rad)
                         - 0.1764f  * cosf(term3Rad);
    return declinationDeg;
}

float elevation(float hourAngleDeg, float latitudeDeg, float declinationDeg)
{
    float hourAngleRad   = degToRad(hourAngleDeg);
    float latitudeRad    = degToRad(latitudeDeg);
    float declinationRad = degToRad(declinationDeg);
    float elevationRad   = asinf(cosf(hourAngleRad) * cosf(latitudeRad) * cosf(declinationRad)
                               + sinf(latitudeRad) * sinf(declinationRad));
    float elevationDeg   = radToDeg(elevationRad);
    return elevationDeg;
}

float azimuth(int dayNumber, float latitudeDeg, float trueSolarMin)
{
    float latitudeRad    = degToRad(latitudeDeg);
    float hourAngleDeg   = hourAngle(trueSolarMin);
    float hourAngleRad   = degToRad(hourAngleDeg);
    float declinationDeg = declination(dayNumber);
    float declinationRad = degToRad(declinationDeg);
    float elevationRad   = asinf(cosf(hourAngleRad) * cosf(latitudeRad) * cosf(declinationRad)
                               + sinf(latitudeRad) * sinf(declinationRad));

    float divisor = cosf(elevationRad) * cosf(latitudeRad);
    if (fabsf(divisor) < AZIMUTH_DIVISOR_MIN) {
        return NAN;
    }

    float cosArgument = (sinf(elevationRad) * sinf(latitudeRad) - sinf(declinationRad)) / divisor;
    if (cosArgument > 1.0f) {
        cosArgument = 1.0f;
    }
    if (cosArgument < -1.0f) {
        cosArgument = -1.0f;
    }

    float azimuthRad;
    if (trueSolarMin < 12.0f * 60.0f) {
        azimuthRad = (float)M_PI - acosf(cosArgument);
    } else {
        azimuthRad = (float)M_PI + acosf(cosArgument);
    }
    float azimuthDeg = radToDeg(azimuthRad);
    return azimuthDeg;
}