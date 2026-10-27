#include <math.h>
#include "HoehenWinkel.h"
#include "grad2rad.h"
#include "rad2grad.h"

// From MATLAB: gamma = asin(cos(beta_r)*cos(phi_r)*cos(delta_r)+sin(phi_r)*sin(delta_r));
float HoehenWinkel(float beta, float phi, float delta)
{
    float beta_r  = grad2rad(beta);
    float phi_r   = grad2rad(phi);
    float delta_r = grad2rad(delta);
    float gamma   = asinf(cosf(beta_r) * cosf(phi_r) * cosf(delta_r) + sinf(phi_r) * sinf(delta_r));
    gamma = rad2grad(gamma);
    return gamma;
}

/* Notes
 * - gamma is the height of the sun above the horizon in degrees.
 *   Negative means the sun is below the horizon (night).
 * - All inputs are in degrees. cos/sin need radians, so grad2rad first.
 * - asinf gives radians, so rad2grad at the end.
 * - asinf only works for values from -1 to 1. At our latitude the
 *   value stays far below 1, so no guard is needed here.
 */