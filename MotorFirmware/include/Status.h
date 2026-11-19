#ifndef STATUS_H
#define STATUS_H

#include <stdint.h>

const uint8_t STATUS_IMU_ON   = 0x01u;
const uint8_t STATUS_MOVING   = 0x02u;
const uint8_t STATUS_READINGS = 0x04u;

uint8_t statusByte(bool imuSwitchOn, bool stepperMoving, bool readingsReady);

#endif