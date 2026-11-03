/**
 * @file MotorTarget.cpp
 * @brief Implementation of the angle-to-byte conversion and the park rule
 *
 * Parking at night avoids the jump of the azimuth at midnight (about 346 to
 * 2 deg), which would turn the panel about 344 deg backwards and could
 * damage the cables. The park position is the start position of the right
 * Nano, so after a power cut at night the real and the assumed position
 * still match.
 *
 * @author Mohamed Sellami
 * @date 2026-11-03
 *
 */
#include <math.h>
#include "MotorTarget.h"

/******************************************************************************
 Constants
******************************************************************************/
/* Park position = start position of MotorFirmware: 90 * 2 = 180 deg (South), 0 deg (vertical). */
static const uint8_t PARK_AZIMUTH_BYTE   = 90;
static const uint8_t PARK_ELEVATION_BYTE = 0;

/*
 * Safe azimuth window, deg. A year scan for Weingarten gives 53.7..306.3 deg
 * during the day, so this only trips if something is broken (e.g. NaN).
 * It never contains North (0/360), where the azimuth jumps.
 */
static const float AZIMUTH_SAFE_MIN_DEG = 50.0f;
static const float AZIMUTH_SAFE_MAX_DEG = 310.0f;

/******************************************************************************
 Function Definitions
******************************************************************************/
uint8_t toUint8(float value)
{
    /*
     * Check before converting: a negative or NaN float put into a uint8_t is
     * undefined behaviour. !(value > 0) is also true for NaN, where value <= 0
     * is not.
     */
    if (!(value > 0.0f))   return 0;
    if (value >= 255.0f)   return 255;
    return (uint8_t)roundf(value);
}

bool targetBytes(float azimuthDeg, float elevationDeg, uint8_t &azimuthByte, uint8_t &elevationByte)
{
    /* Written as !(in range) so that NaN also counts as night or unsafe. */
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