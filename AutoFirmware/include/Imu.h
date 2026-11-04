#ifndef IMU_H
#define IMU_H

#include <stdint.h>

struct ImuRaw {
    int16_t accelX;
    int16_t accelY;
    int16_t magX;
    int16_t magY;
    int16_t magZ;
};

bool imuWhoAmI(uint8_t &id);

bool imuBegin();

bool imuRead(ImuRaw &raw);

#endif