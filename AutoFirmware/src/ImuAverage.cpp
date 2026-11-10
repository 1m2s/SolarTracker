#include "ImuAverage.h"

static const uint16_t MAX_GOOD_READS = 65535u;

static int16_t roundedAverage(int32_t sum, uint16_t goodReads)
{
    int32_t divisor  = (int32_t)goodReads;
    int32_t halfStep = divisor / 2;
    if (sum >= 0) {
        return (int16_t)((sum + halfStep) / divisor);
    }
    return (int16_t)((sum - halfStep) / divisor);
}

void averageReset(ImuAverage &average)
{
    average.sumAccelX = 0;
    average.sumAccelY = 0;
    average.sumMagX   = 0;
    average.sumMagY   = 0;
    average.sumMagZ   = 0;
    average.goodReads = 0u;
}

void averageAdd(ImuAverage &average, bool readOk, const ImuRaw &raw)
{
    if (!readOk) {
        return;
    }
    if (average.goodReads >= MAX_GOOD_READS) {
        return;
    }
    average.sumAccelX += (int32_t)raw.accelX;
    average.sumAccelY += (int32_t)raw.accelY;
    average.sumMagX   += (int32_t)raw.magX;
    average.sumMagY   += (int32_t)raw.magY;
    average.sumMagZ   += (int32_t)raw.magZ;
    average.goodReads  = (uint16_t)(average.goodReads + 1u);
}

bool averageResult(const ImuAverage &average, ImuRaw &result)
{
    if (average.goodReads == 0u) {
        return false;
    }
    result.accelX = roundedAverage(average.sumAccelX, average.goodReads);
    result.accelY = roundedAverage(average.sumAccelY, average.goodReads);
    result.magX   = roundedAverage(average.sumMagX,   average.goodReads);
    result.magY   = roundedAverage(average.sumMagY,   average.goodReads);
    result.magZ   = roundedAverage(average.sumMagZ,   average.goodReads);
    return true;
}