#ifndef SUNPOSITION_H
#define SUNPOSITION_H

void sunPosition(float latitudeDeg, float longitudeDeg, int day, int month,
                 int hour, int minute, bool summerTime,
                 float &azimuthDeg, float &elevationDeg);

#endif