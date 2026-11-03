/**
 * @file AngleConversion.h
 * @brief Convert angles between degrees and radians
 *
 * The MATLAB sun chain works in degrees, but cosf() and sinf() need radians.
 * These two functions convert between them.
 *
 * From MATLAB: grad2rad.m, rad2grad.m
 *
 * Target: Arduino Nano (ATmega328P) and PC (host tests). Level: L2.
 *
 * @author Mohamed Sellami
 * @date 2026-11-03
 *
 */
#ifndef ANGLECONVERSION_H
#define ANGLECONVERSION_H

/******************************************************************************
 Function Declarations
******************************************************************************/
/**
 * @brief Convert degrees to radians
 *
 * From MATLAB: grad2rad.m
 *
 * @param[in] angleDeg  Angle, deg. Any finite value.
 *
 * @return The same angle in rad.
 */
float degToRad(float angleDeg);

/**
 * @brief Convert radians to degrees
 *
 * From MATLAB: rad2grad.m
 *
 * @param[in] angleRad  Angle, rad. Any finite value.
 *
 * @return The same angle in deg.
 */
float radToDeg(float angleRad);

#endif /* ANGLECONVERSION_H */