#ifndef SOLARTIME_H
#define SOLARTIME_H

int dayOfYear(int month, int day);

int cetMinutes(int hour, int minute, bool summerTime);

float meanLocalMinutes(float cetMin, float longitudeDeg);

float equationOfTime(float yearAngleDeg);

float hourAngle(float trueSolarMin);

#endif