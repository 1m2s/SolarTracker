#include "Heading.h"
#include <math.h>

// Calibration: x, y from test_compass_cal + fit_compass.py (2 Nov 2026, 3 runs),
// z from CurrentFirmware (2020)
const float MAG_SCALE = 1200.0f / 4096.0f;
const float MX_CENTRE = 2.08f;
const float MY_CENTRE = 14.05f;
const float MZ_CENTRE = -7.0f;

// Oval -> circle (soft iron). det = 1, so only the shape changes.
const float SOFT_XX = 0.9529f;
const float SOFT_XY = 0.1789f;
const float SOFT_YY = 1.0830f;

const float RAD2GRAD = 180.0f / (float)M_PI;

void magCalibrated(int16_t mx, int16_t my, int16_t mz, float &Mx, float &My, float &Mz)
{
    // 1) move the centre to 0
    float x = mx * MAG_SCALE - MX_CENTRE;
    float y = my * MAG_SCALE - MY_CENTRE;

    // 2) stretch the oval back into a circle
    Mx = SOFT_XX * x + SOFT_XY * y;
    My = SOFT_XY * x + SOFT_YY * y;

    Mz = mz * MAG_SCALE - MZ_CENTRE;
}

// Keeps x inside -1..1, so asin never gives NaN.
static float clamp1(float x)
{
    if (x > 1.0f) {
        return 1.0f;
    }
    if (x < -1.0f) {
        return -1.0f;
    }
    if (x != x) {            // NaN: only true for NaN
        return 0.0f;
    }
    return x;
}

float tiltHeading(float Ax, float Ay, float Mx, float My, float Mz)
{
    float pitch = asinf(clamp1(-Ax));                   // getpitch()
    float roll  = asinf(clamp1(Ay / cosf(pitch)));      // getroll()

    // getTiltHeading()
    float xh = Mx * cosf(pitch) + Mz * sinf(pitch);
    float yh = Mx * sinf(roll) * sinf(pitch) + My * cosf(roll)
             - Mz * sinf(roll) * cosf(pitch);

    float h = atan2f(yh, xh) * RAD2GRAD;
    if (yh < 0) {
        h += 360.0f;
    }
    h = 360.0f - h;

    h = fmodf(h, 360.0f);    // see note 4
    if (h < 0) {
        h += 360.0f;
    }
    return h;
}

/* Notes
1) tiltHeading() is the same math as getpitch(), getroll() and
   getTiltHeading() in Photovoltaiknachfuehrung.ino (CurrentFirmware).
   Same scale 1200 / 4096.

2) What the result means: the compass direction of the sensor's x axis.
   Level board, field along +x -> 0 deg, along -y -> 90 deg,
   along -x -> 180 deg, along +y -> 270 deg. test_heading checks this.

3) Tilt: pitch and roll come from the accelerometer, so a tilted base
   still gives the right heading. On a level base pitch = roll = 0
   and the formula becomes h = 360 - atan2(My, Mx).

4) Two small changes to the original code, both on the edges:
   - The old code could give 360 instead of 0 (same direction).
     fmodf keeps the result in 0 <= h < 360.
   - asin of a value just above 1 (sensor noise) gives NaN.
     clamp1 cuts it to 1 first. Same idea as the acos clamp in Azimut.

5) Calibration (magCalibrated), two steps:
   - Centre: turning the tracker should draw a circle around 0.
     The real circle sits at MX_CENTRE, MY_CENTRE, so that is subtracted.
   - Oval: iron near the sensor (probably the stepper motor) squashes
     the circle into an oval (about 19 : 13). SOFT_XX, SOFT_XY, SOFT_YY
     stretch it back. Without this step, headings in the middle of a
     turn are off by up to about 11 deg.
   All five numbers come from fit_compass.py (test/setup/test_compass_cal),
   from three exact 180 deg turns. Run it again if the tracker gets new
   parts near the sensor or the readings look wrong.

6) Not checked yet: a 90 deg position. fit_compass.py can only check
   the start and end of a turn. setup/test_compass at 0, 90 and 180 deg
   is the next check in the lab.

7) MZ_CENTRE is still the 2020 value. A flat turn cannot measure it,
   and on a level base z hardly changes the heading.
*/