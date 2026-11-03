/**
 * @file SunAngles.h
 * @brief Where the sun is in the sky: declination, elevation and azimuth
 *
 * Second half of the MATLAB sun chain. Works with the day of the year and
 * the times from SolarTime.h and gives the two angles the tracker follows:
 * elevation and azimuth. All angles are in deg.
 *
 * From MATLAB: Deklination.m, HoehenWinkel.m, Azimut.m
 *
 * Target: Arduino Nano (ATmega328P) and PC (host tests). Level: L2.
 *
 * @author Mohamed Sellami
 * @date 2026-11-03
 *
 */
#ifndef SUNANGLES_H
#define SUNANGLES_H

/******************************************************************************
 Function Declarations
******************************************************************************/
/**
 * @brief Declination of the sun: its angle above or below the equator
 *
 * From MATLAB: Deklination.m
 *
 * @param[in] dayNumber  Day of the year, 1..365 (from dayOfYear).
 *
 * @return Declination, deg, -23.42..+23.45: about +23.4 in June,
 *         0 in March and September, -23.4 in December.
 */
float declination(int dayNumber);

/**
 * @brief Elevation of the sun: its height above the horizon
 *
 * From MATLAB: HoehenWinkel.m
 *
 * @param[in] hourAngleDeg    Hour angle, deg (from hourAngle).
 * @param[in] latitudeDeg     Latitude, deg North, -90..90 (47.81 for Weingarten).
 * @param[in] declinationDeg  Declination, deg (from declination).
 *
 * @return Elevation, deg, -90..90. Negative = below the horizon (night).
 */
float elevation(float hourAngleDeg, float latitudeDeg, float declinationDeg);

/**
 * @brief Azimuth of the sun: its compass direction
 *
 * From MATLAB: Azimut.m
 *
 * @param[in] dayNumber     Day of the year, 1..365 (from dayOfYear).
 * @param[in] latitudeDeg   Latitude, deg North, -90..90.
 * @param[in] trueSolarMin  True local time, minutes since 0:00.
 *
 * @return Azimuth, deg, 0..360, measured from North: 90 = East,
 *         180 = South, 270 = West. NAN when the sun is within about
 *         0.06 deg of straight overhead (only possible between the tropics),
 *         because the azimuth has no meaning there.
 */
float azimuth(int dayNumber, float latitudeDeg, float trueSolarMin);

#endif /* SUNANGLES_H */