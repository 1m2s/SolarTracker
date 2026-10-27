#include "SonnenStand.h"
#include "MEZ.h"
#include "MOZ.h"
#include "JahresTag.h"
#include "Zeitgleichung.h"
#include "Deklination.h"
#include "Azimut.h"
#include "StundenWinkel.h"
#include "HoehenWinkel.h"

// From MATLAB: [alpha,gamma] = SonnenStand(Breite,Laenge,Tag,Monat,Stunde,Minute,tf)
void SonnenStand(float Breite, float Laenge, int Tag, int Monat,
                 int Stunde, int Minute, int tf,
                 float &alpha, float &gamma)
{
    int   Min_MEZ = MEZ(Stunde, Minute, tf);
    float Min_MOZ = MOZ(Min_MEZ, Laenge);

    int   d       = JahresTag(Monat, Tag);
    float Jg      = d / 365.0f * 360.0f;
    float zgl     = Zeitgleichung(Jg);
    float Min_WOZ = Min_MOZ + zgl;

    float delta   = Deklination(d);
    alpha         = Azimut(d, Breite, Min_WOZ);
    float beta    = StundenWinkel(Min_WOZ);
    gamma         = HoehenWinkel(beta, Breite, delta);
}

/* Notes
 * - Gives back two values (alpha and gamma), so they are passed
 *   in with & and the function fills them.
 * - alpha = compass direction (180 = South), gamma = height above horizon.
 * - 365.0f, not 365. d is an int, so d/365 would be 0.
 * - Careful: Tag/Monat order here, but JahresTag takes Monat/Tag.
 */