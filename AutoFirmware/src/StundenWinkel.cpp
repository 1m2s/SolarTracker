#include "StundenWinkel.h"

// From MATLAB: WOZ_h = Min_WOZ/60; beta = (12-WOZ_h)*15;
float StundenWinkel(float Min_WOZ)
{
    float WOZ_h = Min_WOZ / 60.0f;      // Minuten -> Stunden mit Dezimalbruch
    float beta  = (12.0f - WOZ_h) * 15.0f; // beta in Grad, 15 Grad = 1 Stunde
    return beta;
}

/* Notes
 * - The sun moves 15 degrees per hour.
 * - beta is positive in the morning, 0 at noon, negative in the afternoon.
 * - 60.0f (not 60) so the division keeps the decimals.
 */