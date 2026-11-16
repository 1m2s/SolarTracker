#ifndef IMUAVERAGE_H
#define IMUAVERAGE_H

#include <stdint.h>
#include "Imu.h"

struct ImuAverage {
    int32_t  sumAccelX;
    int32_t  sumAccelY;
    int32_t  sumMagX;
    int32_t  sumMagY;
    int32_t  sumMagZ;
    uint16_t goodReads;
};

void averageReset(ImuAverage &average);

void averageAdd(ImuAverage &average, bool readOk, const ImuRaw &raw);

bool averageResult(const ImuAverage &average, ImuRaw &result);

#endif