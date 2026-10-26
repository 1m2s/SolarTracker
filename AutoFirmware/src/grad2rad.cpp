#include "grad2rad.h"
#include <math.h>

//From matlab: rad = grad * pi / 180
float grad2rad(float grad) 
{
    return grad * (float)M_PI / 180.0f;
}

/* Notes
1)M_PI is a constant defined in math.h 
that represents the value of pi

2)why ive casted M_PI to float?
M_PI is a double constant. Without the cast, grad would be
promoted to double and the multiplication done in double.
The cast keeps the whole calculation in float.

3)why ive used 180.0f ?
because with no f it would be treated as a double constant

4)why float instead of double?
The compiler (avr-gcc 7.3.0) makes double the same size as
float: 32 bit (checked on the board: sizeof(double) == 4).
So writing double would only pretend to have more precision.
But 32-bit float resolves angles to ~0.00004 deg; the tracker
only moves in 1-2 deg steps, so float is more than precise enough.

*/