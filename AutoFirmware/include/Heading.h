#ifndef HEADING_H
#define HEADING_H

#include <stdint.h>

// Raw compass values (AK8963) -> calibrated magnetic field.
// Uses the calibration from fit_compass.py (centre + oval).
void magCalibrated(int16_t magRawX, int16_t magRawY, int16_t magRawZ, float &magX, float &magY, float &magZ);

// Tilt-compensated heading of the IMU in degrees, 0 <= heading < 360.
// accelX, accelY: acceleration in g (raw / 16384).
// magX, magY, magZ: calibrated field from magCalibrated().
float tiltHeading(float accelX, float accelY, float magX, float magY, float magZ);

#endif