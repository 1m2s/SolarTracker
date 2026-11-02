#include "Heading.h"
#include <math.h>

// Calibration from Photovoltaiknachfuehrung.ino (Weingarten, 2020)
const float MAG_SCALE = 1200.0f / 4096.0f;
const float MX_CENTRE = 0.0f;
const float MY_CENTRE = 13.0f;
const float MZ_CENTRE = -7.0f;

const float RAD2GRAD = 180.0f / (float)M_PI;

void magCalibrated(int16_t mx, int16_t my, int16_t mz, float &Mx, float &My, float &Mz)
{
    Mx = mx * MAG_SCALE - MX_CENTRE;
    My = my * MAG_SCALE - MY_CENTRE;
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
1) Same math as getCompassDate_calibrated(), getpitch(), getroll()
   and getTiltHeading() in Photovoltaiknachfuehrung.ino.
   Same offsets (0, 13, -7) and same scale 1200 / 4096, because the
   CurrentFirmware calibration only fits its own settings.

2) What the result means: the compass direction of the sensor's x axis.
   Level board, field along +x -> 0 deg, along -y -> 90 deg,
   along -x -> 180 deg, along +y -> 270 deg. test_heading checks this.
   Which way that is on the real tracker is checked in test_compass.

3) Tilt: pitch and roll come from the accelerometer, so a tilted base
   still gives the right heading. On a level base pitch = roll = 0
   and the formula becomes h = 360 - atan2(My, Mx).

4) Two small changes to the original code, both on the edges:
   - The old code could give 360 instead of 0 (same direction).
     fmodf keeps the result in 0 <= h < 360.
   - asin of a value just above 1 (sensor noise) gives NaN.
     clamp1 cuts it to 1 first. Same idea as the acos clamp in Azimut.
*/