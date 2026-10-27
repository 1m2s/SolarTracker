#include <math.h>
#include "Azimut.h"
#include "grad2rad.h"
#include "rad2grad.h"
#include "StundenWinkel.h"
#include "Deklination.h"

// From MATLAB: alpha = pi -/+ acos((sin(rgamma)*sin(rphi)-sin(rdelta))/(cos(rgamma)*cos(rphi)))
float Azimut(int d, float phi, float WOZ)
{
    float rphi   = grad2rad(phi);
    float beta   = StundenWinkel(WOZ);
    float rbeta  = grad2rad(beta);
    float delta  = Deklination(d);
    float rdelta = grad2rad(delta);
    float rgamma = asinf(cosf(rbeta) * cosf(rphi) * cosf(rdelta) + sinf(rphi) * sinf(rdelta));

    float x = (sinf(rgamma) * sinf(rphi) - sinf(rdelta)) / (cosf(rgamma) * cosf(rphi));
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
 */