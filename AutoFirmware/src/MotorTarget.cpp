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
static const uint8_t PARK_A8 = 90;
static const uint8_t PARK_G8 = 0;

/*
 * Safe azimuth window, deg. A year scan for Weingarten gives 53.7..306.3 deg
 * during the day, so this only trips if something is broken (e.g. NaN).
 * It never contains North (0/360), where the azimuth jumps.
 */
static const float AZ_MIN = 50.0f;
static const float AZ_MAX = 310.0f;

/******************************************************************************
 Function Definitions
******************************************************************************/
uint8_t toUint8(float x)
{
    /*
     * Check before converting: a negative or NaN float put into a uint8_t is
     * undefined behaviour. !(x > 0) is also true for NaN, where x <= 0 is not.
     */
    if (!(x > 0.0f))   return 0;
    if (x >= 255.0f)   return 255;
    return (uint8_t)roundf(x);
}

bool targetBytes(float alpha, float gamma, uint8_t &a8, uint8_t &g8)
{
    /* Written as !(in range) so that NaN also counts as night or unsafe. */
    bool night  = !(gamma > 0.0f);
    bool unsafe = !(alpha >= AZ_MIN && alpha <= AZ_MAX);

    if (night || unsafe) {
        a8 = PARK_A8;
        g8 = PARK_G8;
        return true;
    }

    a8 = toUint8(alpha / 2.0f);
    g8 = toUint8(gamma);
    return false;
}