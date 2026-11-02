#ifndef HEADING_H
#define HEADING_H

#include <stdint.h>

// Raw compass values (AK8963) -> calibrated magnetic field.
// Uses the calibration from fit_compass.py (centre + oval).
void magCalibrated(int16_t mx, int16_t my, int16_t mz, float &Mx, float &My, float &Mz);

// Tilt-compensated heading of the IMU in degrees, 0 <= h < 360.
// Ax, Ay: acceleration in g (raw / 16384).
// Mx, My, Mz: calibrated field from magCalibrated().
float tiltHeading(float Ax, float Ay, float Mx, float My, float Mz);

#endif