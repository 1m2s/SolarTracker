#ifndef MOTION_H
#define MOTION_H

#include <stdint.h>

// Stepper: which way to step toward the target azimuth.
// off1 = how far the stepper has turned since power-on, in degrees.
// Gives back +1, -1 or 0 (close enough, don't move).
int8_t stepDirection(float azi, float off1);

// Servo: moves the elevation 1 degree toward the target.
// Gives back the new off2.
float servoStep(float gho, float off2);

#endif