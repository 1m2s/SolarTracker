/**
 * @file SunPosition.cpp
 * @brief Implementation of the sun position entry point
 *
 * Calls the MATLAB chain in the same order as SonnenStand.m. The int-to-float
 * casts are exact: dayNumber (1..365) and cetMin (-60..1439) fit a float
 * without loss.
 *
 * From MATLAB: SonnenStand.m
 *
 * @author Mohamed Sellami
 * @date 2026-11-03
 *
 */
#include "SunPosition.h"
#include "SolarTime.h"
#include "SunAngles.h"

/******************************************************************************
 Function Definitions
******************************************************************************/
/* From MATLAB: SonnenStand.m */
void sunPosition(float latitudeDeg, float longitudeDeg, int day, int month,
                 int hour, int minute, int summerTimeFlag,
                 float &azimuthDeg, float &elevationDeg)
{
    int   cetMin            = cetMinutes(hour, minute, summerTimeFlag);
    float meanLocalMin      = meanLocalMinutes((float)cetMin, longitudeDeg);

    int   dayNumber         = dayOfYear(month, day);
    /* 365.0f, not 365: dayNumber is an int, so dayNumber / 365 would be 0. */
    float yearAngleDeg      = (float)dayNumber / 365.0f * 360.0f;
    float equationOfTimeMin = equationOfTime(yearAngleDeg);
    float trueSolarMin      = meanLocalMin + equationOfTimeMin;

    float declinationDeg    = declination(dayNumber);
    azimuthDeg              = azimuth(dayNumber, latitudeDeg, trueSolarMin);
    float hourAngleDeg      = hourAngle(trueSolarMin);
    elevationDeg            = elevation(hourAngleDeg, latitudeDeg, declinationDeg);
}