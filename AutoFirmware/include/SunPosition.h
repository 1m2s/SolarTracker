/**
 * @file SunPosition.h
 * @brief Sun position for a place and a clock time: the entry point
 *
 * Runs the whole MATLAB sun chain (SolarTime.h, then SunAngles.h) and gives
 * the two angles the tracker follows. main.cpp only needs this header.
 *
 * From MATLAB: SonnenStand.m
 *
 * Target: Arduino Nano (ATmega328P) and PC (host tests). Level: L2.
 *
 * @author Mohamed Sellami
 * @date 2026-11-03
 *
 */
#ifndef SUNPOSITION_H
#define SUNPOSITION_H

/******************************************************************************
 Function Declarations
******************************************************************************/
/**
 * @brief Compute azimuth and elevation of the sun
 *
 * Given a place and a clock time, computes where the sun is. MATLAB returns
 * [alpha, gamma]; C++ cannot return two values, so they are written into the
 * two references.
 *
 * From MATLAB: SonnenStand.m
 *
 * @param[in]  Breite  Latitude, deg North (47.81 for Weingarten).
 * @param[in]  Laenge  Longitude, deg East (9.66 for Weingarten).
 * @param[in]  Tag     Day of the month, 1..31.
 * @param[in]  Monat   Month, 1..12.
 * @param[in]  Stunde  Hour of the clock, 0..23.
 * @param[in]  Minute  Minute of the clock, 0..59.
 * @param[in]  tf      0 = the clock shows MEZ (winter time), 1 = summer time.
 * @param[out] alpha   Azimuth, deg, 0..360 (180 = South), or NAN when the
 *                     sun is straight overhead. Always written.
 * @param[out] gamma   Elevation, deg, -90..90, negative at night.
 *                     Always written.
 *
 * @note The order is Tag, Monat here, but Monat, Tag in JahresTag (same as
 *       MATLAB).
 */
void SonnenStand(float Breite, float Laenge, int Tag, int Monat,
                 int Stunde, int Minute, int tf,
                 float &alpha, float &gamma);

#endif /* SUNPOSITION_H */