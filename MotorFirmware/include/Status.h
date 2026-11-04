#ifndef STATUS_H
#define STATUS_H

#include <stdint.h>

// Bits of the status byte the left Nano reads with Wire.requestFrom(0x19, 1).
const uint8_t STATUS_IMU_ON = 0x01u;   // bit 0: IMU switch AN
const uint8_t STATUS_MOVING = 0x02u;   // bit 1: stepper still has steps to go

// Puts the two facts together into one byte (0, 1, 2 or 3).
uint8_t statusByte(bool imuSwitchOn, bool stepperMoving);

#endif