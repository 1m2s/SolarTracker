#include <math.h>
#include "Azimut.h"
#include "grad2rad.h"
#include "rad2grad.h"
#include "StundenWinkel.h"
#include "Deklination.h"

// Smallest divisor we accept. Below it the sun is within about 0.06 deg
// of straight overhead, where the azimuth has no meaning.
const float AZ_DEN_MIN = 1.0e-3f;

// From MATLAB: alpha = pi -/+ acos((sin(rgamma)*sin(rphi)-sin(rdelta))/(cos(rgamma)*cos(rphi)))
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
        return NAN;                      // no azimuth -> targetBytes parks
    }

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

/* Notes
 * - alpha is the compass direction of the sun. 90 = East,
 *   180 = South, 270 = West.
 * - MATLAB loops over many times. Here we only do one time,
 *   so no loop.
 * - The gamma line from MATLAB is not used, so it is left out.
 * - x is kept between -1 and 1. At noon x should be exactly 1,
 *   but rounding can make it a bit bigger. Then acosf gives NaN.
 *   MATLAB has the same problem (gives a complex number).
 * - den is 0 when the sun is straight overhead (gamma = 90 deg).
 *   In Weingarten the sun gets at most 65.6 deg high, so den never
 *   goes below 0.27 and the guard never fires. It is there because
 *   a division must be checked, and it would matter if Breite were
 *   changed to a place between the tropics.
 */