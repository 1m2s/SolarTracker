#include <math.h>
#include "MotorTarget.h"

static const uint8_t PARK_AZIMUTH_BYTE   = 90u;
static const uint8_t PARK_ELEVATION_BYTE = 0u;

static const float AZIMUTH_SAFE_MIN_DEG = 50.0f;
static const float AZIMUTH_SAFE_MAX_DEG = 310.0f;

uint8_t toUint8(float value)
{
    if (!(value > 0.0f)) {
        return 0u;
    }
    if (value >= 255.0f) {
        return 255u;
    }
    return (uint8_t)roundf(value);
}

bool targetBytes(float azimuthDeg, float elevationDeg, uint8_t &azimuthByte, uint8_t &elevationByte)
{
    bool night  = !(elevationDeg > 0.0f);
    bool unsafe = !(azimuthDeg >= AZIMUTH_SAFE_MIN_DEG && azimuthDeg <= AZIMUTH_SAFE_MAX_DEG);

    if (night || unsafe) {
        azimuthByte   = PARK_AZIMUTH_BYTE;
        elevationByte = PARK_ELEVATION_BYTE;
        return true;
    }

    azimuthByte   = toUint8(azimuthDeg / 2.0f);
    elevationByte = toUint8(elevationDeg);
    return false;
}