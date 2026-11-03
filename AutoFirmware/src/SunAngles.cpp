/**
 * @file SunAngles.cpp
 * @brief Implementation of the angle half of the MATLAB sun chain
 *
 * Line-by-line ports of the MATLAB files. The inputs are in deg; cosf, sinf,
 * asinf and acosf work in rad, so every angle goes through degToRad first
 * and the results through radToDeg at the end.
 *
 * From MATLAB: Deklination.m, HoehenWinkel.m, Azimut.m
 *
 * @author Mohamed Sellami
 * @date 2026-11-03
 *
 */
#include <math.h>
#include "SunAngles.h"
#include "SolarTime.h"
#include "AngleConversion.h"

/******************************************************************************
 Constants
******************************************************************************/
/*
 * Smallest divisor azimuth accepts (A9). Below it the sun is within about
 * 0.06 deg of straight overhead. In Weingarten the sun gets at most 65.6 deg
 * high, so the divisor never drops below 0.277 and this never trips there.
 */
static const float AZIMUTH_DIVISOR_MIN = 1.0e-3f;

/******************************************************************************
 Function Definitions
******************************************************************************/
/* From MATLAB: Deklination.m */
float declination(int dayNumber)
{
    float yearAngleDeg   = 360.0f * (float)dayNumber / 365.0f;
    float term1Rad       = degToRad(yearAngleDeg + 9.1f);
    float term2Rad       = degToRad(2.0f * yearAngleDeg + 5.4f);
    float term3Rad       = degToRad(3.0f * yearAngleDeg + 26.0f);
    float declinationDeg = 0.3948f - 23.2559f * cosf(term1Rad) - 0.391f * cosf(term2Rad) - 0.1764f * cosf(term3Rad);
    return declinationDeg;
}

/* From MATLAB: HoehenWinkel.m */
float elevation(float hourAngleDeg, float latitudeDeg, float declinationDeg)
{
    float hourAngleRad   = degToRad(hourAngleDeg);
    float latitudeRad    = degToRad(latitudeDeg);
    float declinationRad = degToRad(declinationDeg);
    /*
     * The asinf argument is sin(elevation): at most sin(65.6 deg) = 0.91 in
     * Weingarten, so it stays inside -1..1 there and needs no clamp.
     */
    float elevationRad   = asinf(cosf(hourAngleRad) * cosf(latitudeRad) * cosf(declinationRad) + sinf(latitudeRad) * sinf(declinationRad));
    float elevationDeg = radToDeg(elevationRad);
    return elevationDeg;
}

/*
 * From MATLAB: Azimut.m. MATLAB loops over many times; here it is one time,
 * so there is no loop. The MATLAB gamma line is never used and is left out.
 */
float azimuth(int dayNumber, float latitudeDeg, float trueSolarMin)
{
    float latitudeRad    = degToRad(latitudeDeg);
    float hourAngleDeg   = hourAngle(trueSolarMin);
    float hourAngleRad   = degToRad(hourAngleDeg);
    float declinationDeg = declination(dayNumber);
    float declinationRad = degToRad(declinationDeg);
    float elevationRad   = asinf(cosf(hourAngleRad) * cosf(latitudeRad) * cosf(declinationRad) + sinf(latitudeRad) * sinf(declinationRad));

    float divisor = cosf(elevationRad) * cosf(latitudeRad);
    if (fabsf(divisor) < AZIMUTH_DIVISOR_MIN) {
        return NAN;
    }

    /*
     * At noon cosArgument is exactly 1 in theory, but rounding can push it a
     * bit over, and acosf would give NaN. MATLAB has the same problem
     * (complex result).
     */
    float cosArgument = (sinf(elevationRad) * sinf(latitudeRad) - sinf(declinationRad)) / divisor;
    if (cosArgument > 1.0f)  cosArgument = 1.0f;
    if (cosArgument < -1.0f) cosArgument = -1.0f;

    float azimuthRad;
    if (trueSolarMin < 12.0f * 60.0f) {
        azimuthRad = (float)M_PI - acosf(cosArgument);
    } else {
        azimuthRad = (float)M_PI + acosf(cosArgument);
    }
    float azimuthDeg = radToDeg(azimuthRad);
    return azimuthDeg;
}