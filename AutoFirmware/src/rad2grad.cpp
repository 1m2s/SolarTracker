#include <math.h>
#include "rad2grad.h"

// From MATLAB: grad = rad/pi*180;
float rad2grad(float rad)
{
    return rad / (float)M_PI * 180.0f;
}

/* Notes
 * - Same order as MATLAB: divide by pi first, then multiply by 180.
 */