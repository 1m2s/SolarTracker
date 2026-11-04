#include "SunPosition.h"
#include "SolarTime.h"
#include "SunAngles.h"

void sunPosition(float latitudeDeg, float longitudeDeg, int day, int month,
                 int hour, int minute, bool summerTime,
                 float &azimuthDeg, float &elevationDeg)
{
    int   cetMin            = cetMinutes(hour, minute, summerTime);
    float meanLocalMin      = meanLocalMinutes((float)cetMin, longitudeDeg);

    int   dayNumber         = dayOfYear(month, day);
    float yearAngleDeg      = (float)dayNumber / 365.0f * 360.0f;
    float equationOfTimeMin = equationOfTime(yearAngleDeg);
    float trueSolarMin      = meanLocalMin + equationOfTimeMin;

    float declinationDeg    = declination(dayNumber);
    azimuthDeg              = azimuth(dayNumber, latitudeDeg, trueSolarMin);
    float hourAngleDeg      = hourAngle(trueSolarMin);
    elevationDeg            = elevation(hourAngleDeg, latitudeDeg, declinationDeg);
}