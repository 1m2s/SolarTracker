#ifndef MOTORTARGET_H
#define MOTORTARGET_H

#include <stdint.h>

uint8_t toUint8(float value);

bool targetBytes(float azimuthDeg, float elevationDeg, uint8_t &azimuthByte, uint8_t &elevationByte);
/* 
Input: 2 copies of the azimuth and elevation angles in degrees, and 2 references to uint8_t variables to hold the output bytes.
Ouput: 2 Bytes representing the azimuth and elevation angles, and a boolean indicating whether the target is in a safe position or not.
*/

bool alignBytes(float azimuthDeg, uint8_t &azimuthByte, uint8_t &elevationByte);

#endif