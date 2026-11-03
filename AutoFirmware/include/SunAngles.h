#ifndef SUNANGLES_H
#define SUNANGLES_H

float declination(int dayNumber);

float elevation(float hourAngleDeg, float latitudeDeg, float declinationDeg);

float azimuth(int dayNumber, float latitudeDeg, float trueSolarMin);

#endif