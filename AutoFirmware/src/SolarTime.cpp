/**
 * @file SolarTime.cpp
 * @brief Implementation of the time half of the MATLAB sun chain
 *
 * Each function is a line-by-line port of its MATLAB file, with the same
 * names and the same order of operations. Numbers end in f so the math
 * stays in float.
 *
 * From MATLAB: JahresTag.m, MEZ.m, MOZ.m, Zeitgleichung.m, StundenWinkel.m
 *
 * @author Mohamed Sellami
 * @date 2026-11-03
 *
 */
#include <math.h>
#include "SolarTime.h"
#include "AngleConversion.h"

/******************************************************************************
 Function Definitions
******************************************************************************/
/* From MATLAB: JahresTag.m (MATLAB counts from 1, so MTage(1) is MTage[0]) */
int JahresTag(int Monat, int Tag)
{
    const int MTage[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int d = Tag;

    if (Monat > 1) {
        for (int i = 0; i < Monat - 1; i++) {
            d = d + MTage[i];
        }
    }
    return d;
}

/* From MATLAB: MEZ.m */
int MEZ(int Stunde, int Minute, int tf)
{
    int MEZ_Stunde = Stunde;
    if (tf == 1) {
        MEZ_Stunde = Stunde - 1;
    } else if (tf == 0) {
        MEZ_Stunde = Stunde;
    }

    int MEZ_Minute = Minute;
    int Min_MEZ    = MEZ_Minute + MEZ_Stunde * 60;
    return Min_MEZ;
}

/* From MATLAB: MOZ.m */
float MOZ(float Min_MEZ, float lambda)
{
    float Min_MOZ = Min_MEZ - 4.0f * (15.0f - lambda);
    return Min_MOZ;
}

/* From MATLAB: Zeitgleichung.m (cosf needs rad, MATLAB gives the angles in deg) */
float Zeitgleichung(float Jg)
{
    float a1 = grad2rad(Jg + 85.9f);
    float a2 = grad2rad(2.0f * Jg + 108.9f);
    float a3 = grad2rad(3.0f * Jg + 105.2f);
    float zgl = 0.0066f + 7.3525f * cosf(a1) + 9.9359f * cosf(a2) + 0.3387f * cosf(a3);
    return zgl;
}

/* From MATLAB: StundenWinkel.m */
float StundenWinkel(float Min_WOZ)
{
    float WOZ_h = Min_WOZ / 60.0f;
    float beta  = (12.0f - WOZ_h) * 15.0f;
    return beta;
}