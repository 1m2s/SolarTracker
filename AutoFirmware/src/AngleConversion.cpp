/**
 * @file AngleConversion.cpp
 * @brief Implementation of the degree/radian conversion
 *
 * All math is in float. On the AVR, double is also only 32 bit, so double
 * would not be more precise. M_PI is a double, so it is cast to float to keep
 * the whole calculation in float.
 *
 * From MATLAB: grad2rad.m, rad2grad.m
 *
 * @author Mohamed Sellami
 * @date 2026-11-03
 *
 */
#include <math.h>
#include "AngleConversion.h"

/******************************************************************************
 Function Definitions
******************************************************************************/
/* From MATLAB: grad2rad.m */
float degToRad(float angleDeg)
{
    return angleDeg * (float)M_PI / 180.0f;
}

/* From MATLAB: rad2grad.m (same order: divide by pi first, then times 180) */
float radToDeg(float angleRad)
{
    return angleRad / (float)M_PI * 180.0f;
}