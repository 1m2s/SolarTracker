/**
 * @file MotorTarget.h
 * @brief Turn the sun angles into the two bytes for the right Nano
 *
 * The right Nano (MotorFirmware) takes one byte for the azimuth (in steps of
 * 2 deg) and one for the elevation (in deg). At night, or if an angle
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
 * @param[in] value  Any value, also negative or NaN.
 *
 * @return 0 for value <= 0 and for NaN, 255 for value >= 255, otherwise value
 *         rounded.
 */
uint8_t toUint8(float value);

/**
 * @brief Make the two target bytes for the right Nano, or park at night
 *
 * Given a sun position, writes the bytes the right Nano expects:
 * azimuthByte = azimuthDeg / 2 and elevationByte = elevationDeg, both through
 * toUint8 (same as AusgabeSonnenstand.m). At night or with an unsafe azimuth,
 * writes the park position instead: azimuthByte = 90 (180 deg, South) and
 * elevationByte = 0 (panel vertical).
 *
 * @param[in]  azimuthDeg     Azimuth, deg. Inside 50..310 = safe, anything
 *                            else (also NaN) parks.
 * @param[in]  elevationDeg   Elevation, deg. Above 0 = day, 0 or below (also
 *                            NaN) parks.
 * @param[out] azimuthByte    Azimuth byte, 0..255 (deg / 2). Always written.
 * @param[out] elevationByte  Elevation byte, 0..255 (deg). Always written.
 *
 * @return true if the park position was written, false if the bytes follow
 *         the sun.
 */
bool targetBytes(float azimuthDeg, float elevationDeg, uint8_t &azimuthByte, uint8_t &elevationByte);

#endif /* MOTORTARGET_H */