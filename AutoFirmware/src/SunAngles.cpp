/**
 * @file SunAngles.cpp
 * @brief Implementation of the angle half of the MATLAB sun chain
 *
 * Line-by-line ports of the MATLAB files. The inputs are in deg; cosf, sinf,
 * asinf and acosf work in rad, so every angle goes through grad2rad first
 * and the results through rad2grad at the end.
 *
 * From MATLAB: Deklination.m, HoehenWinkel.m, Azimut.m
 *
 * @author Mohamed Sellami
 * @date 2026-11-03
 *
 */
#include <math.h>
#include "SunAngles.h"
#include "SolarTime.h"
#include "AngleConversion.h"

/******************************************************************************
 Constants
******************************************************************************/
/*
 * Smallest divisor Azimut accepts (A9). Below it the sun is within about
 * 0.06 deg of straight overhead. In Weingarten the sun gets at most 65.6 deg
 * high, so the divisor never drops below 0.277 and this never trips there.
 */
static const float AZ_DEN_MIN = 1.0e-3f;

/******************************************************************************
 Function Definitions
******************************************************************************/
/* From MATLAB: Deklination.m */
float Deklination(int d)
{
    float Js    = 360.0f * (float)d / 365.0f;
    float a1    = grad2rad(Js + 9.1f);
    float a2    = grad2rad(2.0f * Js + 5.4f);
    float a3    = grad2rad(3.0f * Js + 26.0f);
    float delta = 0.3948f - 23.2559f * cosf(a1) - 0.391f * cosf(a2) - 0.1764f * cosf(a3);
    return delta;
}

/* From MATLAB: HoehenWinkel.m */
float HoehenWinkel(float beta, float phi, float delta)
{
    float beta_r  = grad2rad(beta);
    float phi_r   = grad2rad(phi);
    float delta_r = grad2rad(delta);
    /*
     * The asinf argument is sin(elevation): at most sin(65.6 deg) = 0.91 in
     * Weingarten, so it stays inside -1..1 there and needs no clamp.
     */
    float gamma   = asinf(cosf(beta_r) * cosf(phi_r) * cosf(delta_r) + sinf(phi_r) * sinf(delta_r));
    gamma = rad2grad(gamma);
    return gamma;
}

/*
 * From MATLAB: Azimut.m. MATLAB loops over many times; here it is one time,
 * so there is no loop. The MATLAB gamma line is never used and is left out.
 */
float Azimut(int d, float phi, float WOZ)
{
    float rphi   = grad2rad(phi);
    float beta   = StundenWinkel(WOZ);
    float rbeta  = grad2rad(beta);
    float delta  = Deklination(d);
    float rdelta = grad2rad(delta);
    float rgamma = asinf(cosf(rbeta) * cosf(rphi) * cosf(rdelta) + sinf(rphi) * sinf(rdelta));

    float den = cosf(rgamma) * cosf(rphi);
    if (fabsf(den) < AZ_DEN_MIN) {
        return NAN;
    }

    /*
     * At noon x is exactly 1 in theory, but rounding can push it a bit over,
     * and acosf would give NaN. MATLAB has the same problem (complex result).
     */
    float x = (sinf(rgamma) * sinf(rphi) - sinf(rdelta)) / den;
    if (x > 1.0f)  x = 1.0f;
    if (x < -1.0f) x = -1.0f;

    float alpha;
    if (WOZ < 12.0f * 60.0f) {
        alpha = (float)M_PI - acosf(x);
    } else {
        alpha = (float)M_PI + acosf(x);
    }
    alpha = rad2grad(alpha);
    return alpha;
}