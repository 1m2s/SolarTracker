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
 * @param[in]  latitudeDeg     Latitude, deg North (47.81 for Weingarten).
 * @param[in]  longitudeDeg    Longitude, deg East (9.66 for Weingarten).
 * @param[in]  day             Day of the month, 1..31.
 * @param[in]  month           Month, 1..12.
 * @param[in]  hour            Hour of the clock, 0..23.
 * @param[in]  minute          Minute of the clock, 0..59.
 * @param[in]  summerTime      false = the clock shows CET (winter time),
 *                             true = the clock shows summer time (CEST).
 * @param[out] azimuthDeg      Azimuth, deg, 0..360 (180 = South), or NAN when
 *                             the sun is straight overhead. Always written.
 * @param[out] elevationDeg    Elevation, deg, -90..90, negative at night.
 *                             Always written.
 *
 * @note The order is day, month here, but month, day in dayOfYear (same as
 *       MATLAB).
 */
void sunPosition(float latitudeDeg, float longitudeDeg, int day, int month,
                 int hour, int minute, bool summerTime,
                 float &azimuthDeg, float &elevationDeg);

#endif /* SUNPOSITION_H */