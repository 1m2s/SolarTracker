/**
 * @file MotorTarget.h
 * @brief Turn the sun angles into the two bytes for the right Nano
 *
 * The right Nano (MotorFirmware) takes one byte for the azimuth (a8, in steps
 * of 2 deg) and one for the elevation (g8, in deg). At night, or if an angle
 * is unsafe, both bytes are set to the park position instead.
 *
 * Target: Arduino Nano (ATmega328P) and PC (host tests). Level: L2.
 *
 * @author Mohamed Sellami
 * @date 2026-11-03
 *
 */
#ifndef MOTORTARGET_H
#define MOTORTARGET_H

#include <stdint.h>

/******************************************************************************
 Function Declarations
******************************************************************************/
/**
 * @brief Convert a float to a byte like uint8() in MATLAB
 *
 * Rounds to the nearest whole number, then clips to 0..255.
 *
 * @param[in] x  Any value, also negative or NaN.
 *
 * @return 0 for x <= 0 and for NaN, 255 for x >= 255, otherwise x rounded.
 */
uint8_t toUint8(float x);

/**
 * @brief Make the two target bytes for the right Nano, or park at night
 *
 * Given a sun position, writes the bytes the right Nano expects:
 * a8 = alpha / 2 and g8 = gamma, both through toUint8 (same as
 * AusgabeSonnenstand.m). At night or with an unsafe azimuth, writes the park
 * position instead: a8 = 90 (180 deg, South) and g8 = 0 (panel vertical).
 *
 * @param[in]  alpha  Azimuth, deg. Inside 50..310 = safe, anything else
 *                    (also NaN) parks.
 * @param[in]  gamma  Elevation, deg. Above 0 = day, 0 or below (also NaN)
 *                    parks.
 * @param[out] a8     Azimuth byte, 0..255 (deg / 2). Always written.
 * @param[out] g8     Elevation byte, 0..255 (deg). Always written.
 *
 * @return true if the park position was written, false if the bytes follow
 *         the sun.
 */
bool targetBytes(float alpha, float gamma, uint8_t &a8, uint8_t &g8);

#endif /* MOTORTARGET_H */