/**
 * @file SolarTime.h
 * @brief Turn the clock time into sun time and the hour angle
 *
 * First half of the MATLAB sun chain. sunPosition calls these in this order:
 * clock time (CET) -> mean local time -> true local time (using the equation
 * of time) -> hour angle. dayOfYear gives the day of the year that the
 * equation of time needs.
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
 * @param[in] month  Month, 1..12.
 * @param[in] day    Day of the month, 1..31.
 *
 * @return Day of the year, 1..365 (1 = 1 January).
 *
 * @note Leap years are ignored (February always has 28 days), same as MATLAB.
 *       TODO(F3): month is not range-checked yet.
 */
int dayOfYear(int month, int day);

/**
 * @brief Clock time as minutes since 0:00 CET
 *
 * From MATLAB: MEZ.m
 *
 * @param[in] hour        Hour of the clock, 0..23.
 * @param[in] minute      Minute of the clock, 0..59.
 * @param[in] summerTime  false = the clock shows CET (winter time),
 *                        true = the clock shows summer time (CEST).
 *
 * @return Minutes since 0:00 CET, -60..1439.
 *
 * @note In summer time before 1:00 the result is negative (00:30 gives -30,
 *       not 23:30 of the day before). Same as MATLAB.
 */
int cetMinutes(int hour, int minute, bool summerTime);

/**
 * @brief Mean local time from CET
 *
 * CET is the time at 15 deg East. Every degree west of that is 4 min earlier.
 *
 * From MATLAB: MOZ.m
 *
 * @param[in] cetMin        Minutes since 0:00 CET.
 * @param[in] longitudeDeg  Longitude, deg East (9.66 for Weingarten).
 *
 * @return Minutes since 0:00 mean local time.
 */
float meanLocalMinutes(float cetMin, float longitudeDeg);

/**
 * @brief Equation of time: true local time minus mean local time
 *
 * From MATLAB: Zeitgleichung.m
 *
 * @param[in] yearAngleDeg  Day of the year as an angle, deg:
 *                          dayOfYear / 365 * 360, 0..360.
 *
 * @return Correction in min, about -14.4..+16.5 over the year.
 *         True local time = mean local time + this value.
 */
float equationOfTime(float yearAngleDeg);

/**
 * @brief Hour angle of the sun from true local time
 *
 * The sun moves 15 deg per hour and stands in the south at 12:00 true local
 * time.
 *
 * From MATLAB: StundenWinkel.m
 *
 * @param[in] trueSolarMin  Minutes since 0:00 true local time.
 *
 * @return Hour angle, deg: positive in the morning, 0 at noon,
 *         negative in the afternoon.
 */
float hourAngle(float trueSolarMin);

#endif /* SOLARTIME_H */