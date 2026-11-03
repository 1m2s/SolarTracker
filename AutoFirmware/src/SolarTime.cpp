/**
 * @file SolarTime.cpp
 * @brief Implementation of the time half of the MATLAB sun chain
 *
 * Each function is a line-by-line port of its MATLAB file, with the same
 * order of operations. Numbers end in f so the math stays in float.
 *
 * From MATLAB: JahresTag.m, MEZ.m, MOZ.m, Zeitgleichung.m, StundenWinkel.m
 *
 * @author Mohamed Sellami
 * @date 2026-11-03
 *
 */
#include <math.h>
#include "SolarTime.h"
#include "AngleConversion.h"
 
/******************************************************************************
 Function Definitions
******************************************************************************/
/* From MATLAB: JahresTag.m (MATLAB counts from 1, so MTage(1) is daysInMonth[0]) */
int dayOfYear(int month, int day)
{
    const int daysInMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int dayNumber = day;
 
    if (month > 1) {
        for (int monthIndex = 0; monthIndex < month - 1; monthIndex++) {
            dayNumber = dayNumber + daysInMonth[monthIndex];
        }
    }
    return dayNumber;
}
 
/* From MATLAB: MEZ.m (tf == 1 is summerTime == true, tf == 0 is false) */
int cetMinutes(int hour, int minute, bool summerTime)
{
    int cetHour = hour;
    if (summerTime) {
        cetHour = hour - 1;
    }
 
    int cetMinute = minute;
    int cetMin    = cetMinute + cetHour * 60;
    return cetMin;
}
 
/* From MATLAB: MOZ.m */
float meanLocalMinutes(float cetMin, float longitudeDeg)
{
    float meanLocalMin = cetMin - 4.0f * (15.0f - longitudeDeg);
    return meanLocalMin;
}
 
/* From MATLAB: Zeitgleichung.m (cosf needs rad, MATLAB gives the angles in deg) */
float equationOfTime(float yearAngleDeg)
{
    float term1Rad = degToRad(yearAngleDeg + 85.9f);
    float term2Rad = degToRad(2.0f * yearAngleDeg + 108.9f);
    float term3Rad = degToRad(3.0f * yearAngleDeg + 105.2f);
    float equationOfTimeMin = 0.0066f
                            + 7.3525f * cosf(term1Rad)
                            + 9.9359f * cosf(term2Rad)
                            + 0.3387f * cosf(term3Rad);
    return equationOfTimeMin;
}
 
/* From MATLAB: StundenWinkel.m */
float hourAngle(float trueSolarMin)
{
    float trueSolarHours = trueSolarMin / 60.0f;
    float hourAngleDeg   = (12.0f - trueSolarHours) * 15.0f;
    return hourAngleDeg;
}