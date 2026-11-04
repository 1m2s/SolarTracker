#ifndef HEADING_H
#define HEADING_H

#include <stdint.h>

void magCalibrated(int16_t magRawX, int16_t magRawY, int16_t magRawZ, float &magX, float &magY, float &magZ);

float tiltHeading(float accelX, float accelY, float magX, float magY, float magZ);

#endif