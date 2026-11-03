#ifndef MOTORTARGET_H
#define MOTORTARGET_H

#include <stdint.h>

uint8_t toUint8(float value);

bool targetBytes(float azimuthDeg, float elevationDeg, uint8_t &azimuthByte, uint8_t &elevationByte);

#endif