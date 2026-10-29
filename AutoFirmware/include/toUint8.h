#ifndef TOUINT8_H
#define TOUINT8_H

#include <stdint.h>

// float -> uint8_t like MATLAB uint8(): round, then clip to 0..255
uint8_t toUint8(float x);

#endif