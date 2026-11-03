/**
 * @file SunPosition.cpp
 * @brief Implementation of the sun position entry point
 *
 * Calls the MATLAB chain in the same order as SonnenStand.m. The int-to-float
 * casts are exact: d (1..365) and Min_MEZ (-60..1439) fit a float without loss.
 *
 * From MATLAB: SonnenStand.m
 *
 * @author Mohamed Sellami
 * @date 2026-11-03
 *
 */
#include "SunPosition.h"
#include "SolarTime.h"
#include "SunAngles.h"

/******************************************************************************
 Function Definitions
******************************************************************************/
/* From MATLAB: SonnenStand.m */
void SonnenStand(float Breite, float Laenge, int Tag, int Monat,
                 int Stunde, int Minute, int tf,
                 float &alpha, float &gamma)
{
    int   Min_MEZ = MEZ(Stunde, Minute, tf);
    float Min_MOZ = MOZ((float)Min_MEZ, Laenge);

    int   d       = JahresTag(Monat, Tag);
    /* 365.0f, not 365: d is an int, so d / 365 would be 0. */
    float Jg      = (float)d / 365.0f * 360.0f;
    float zgl     = Zeitgleichung(Jg);
    float Min_WOZ = Min_MOZ + zgl;

    float delta   = Deklination(d);
    alpha         = Azimut(d, Breite, Min_WOZ);
    float beta    = StundenWinkel(Min_WOZ);
    gamma         = HoehenWinkel(beta, Breite, delta);
}