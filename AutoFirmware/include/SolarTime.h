/**
 * @file SolarTime.h
 * @brief Turn the clock time into sun time and the hour angle
 *
 * First half of the MATLAB sun chain. SonnenStand calls these in this order:
 * clock time (MEZ) -> mean local time (MOZ) -> true local time (WOZ, using
 * the equation of time) -> hour angle. JahresTag gives the day of the year
 * that the equation of time needs.
 *
 * From MATLAB: JahresTag.m, MEZ.m, MOZ.m, Zeitgleichung.m, StundenWinkel.m
 *
 * Target: Arduino Nano (ATmega328P) and PC (host tests). Level: L2.
 *
 * @author Mohamed Sellami
 * @date 2026-11-03
 *
 */
#ifndef SOLARTIME_H
#define SOLARTIME_H

/******************************************************************************
 Function Declarations
******************************************************************************/
/**
 * @brief Day of the year from month and day
 *
 * From MATLAB: JahresTag.m
 *
 * @param[in] Monat  Month, 1..12.
 * @param[in] Tag    Day of the month, 1..31.
 *
 * @return Day of the year, 1..365 (1 = 1 January).
 *
 * @note Leap years are ignored (February always has 28 days), same as MATLAB.
 *       TODO(F3): Monat is not range-checked yet.
 */
int JahresTag(int Monat, int Tag);

/**
 * @brief Clock time as minutes since 0:00 MEZ
 *
 * From MATLAB: MEZ.m
 *
 * @param[in] Stunde  Hour of the clock, 0..23.
 * @param[in] Minute  Minute of the clock, 0..59.
 * @param[in] tf      0 = the clock shows MEZ (winter time), 1 = summer time.
 *                    Any other value acts like 0.
 *
 * @return Minutes since 0:00 MEZ, -60..1439.
 *
 * @note In summer time before 1:00 the result is negative (00:30 gives -30,
 *       not 23:30 of the day before). Same as MATLAB.
 */
int MEZ(int Stunde, int Minute, int tf);

/**
 * @brief Mean local time (MOZ) from MEZ
 *
 * MEZ is the time at 15 deg East. Every degree west of that is 4 min earlier.
 *
 * From MATLAB: MOZ.m
 *
 * @param[in] Min_MEZ  Minutes since 0:00 MEZ.
 * @param[in] lambda   Longitude, deg East (9.66 for Weingarten).
 *
 * @return Minutes since 0:00 mean local time.
 */
float MOZ(float Min_MEZ, float lambda);

/**
 * @brief Equation of time: true local time minus mean local time
 *
 * From MATLAB: Zeitgleichung.m
 *
 * @param[in] Jg  Day of the year as an angle, deg: JahresTag / 365 * 360,
 *                0..360.
 *
 * @return Correction in min, about -14.4..+16.5 over the year.
 *         True local time WOZ = MOZ + this value.
 */
float Zeitgleichung(float Jg);

/**
 * @brief Hour angle of the sun from true local time
 *
 * The sun moves 15 deg per hour and stands in the south at 12:00 WOZ.
 *
 * From MATLAB: StundenWinkel.m
 *
 * @param[in] Min_WOZ  Minutes since 0:00 true local time.
 *
 * @return Hour angle beta, deg: positive in the morning, 0 at noon,
 *         negative in the afternoon.
 */
float StundenWinkel(float Min_WOZ);

#endif /* SOLARTIME_H */