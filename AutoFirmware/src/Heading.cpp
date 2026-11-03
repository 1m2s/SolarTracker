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

const float DEG_PER_RAD = 180.0f / (float)M_PI;

void magCalibrated(int16_t magRawX, int16_t magRawY, int16_t magRawZ, float &magX, float &magY, float &magZ)
{
    // 1) move the centre to 0
    float centredX = magRawX * MAG_SCALE - MX_CENTRE;
    float centredY = magRawY * MAG_SCALE - MY_CENTRE;

    // 2) stretch the oval back into a circle
    magX = SOFT_XX * centredX + SOFT_XY * centredY;
    magY = SOFT_XY * centredX + SOFT_YY * centredY;

    magZ = magRawZ * MAG_SCALE - MZ_CENTRE;
}

// Keeps value inside -1..1, so asin never gives NaN.
static float clampToUnitRange(float value)
{
    if (value > 1.0f) {
        return 1.0f;
    }
    if (value < -1.0f) {
        return -1.0f;
    }
    if (!(value >= -1.0f)) { // NaN: every compare with NaN is false
        return 0.0f;
    }
    return value;
}

float tiltHeading(float accelX, float accelY, float magX, float magY, float magZ)
{
    float pitchRad = asinf(clampToUnitRange(-accelX));                   // getpitch()
    float rollRad  = asinf(clampToUnitRange(accelY / cosf(pitchRad)));   // getroll()

    // getTiltHeading()
    float horizontalX = magX * cosf(pitchRad) + magZ * sinf(pitchRad);
    float horizontalY = magX * sinf(rollRad) * sinf(pitchRad) + magY * cosf(rollRad)
                      - magZ * sinf(rollRad) * cosf(pitchRad);

    float headingDeg = atan2f(horizontalY, horizontalX) * DEG_PER_RAD;
    if (horizontalY < 0) {
        headingDeg += 360.0f;
    }
    headingDeg = 360.0f - headingDeg;

    headingDeg = fmodf(headingDeg, 360.0f);    // see note 4
    if (headingDeg < 0) {
        headingDeg += 360.0f;
    }
    return headingDeg;
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
   and the formula becomes headingDeg = 360 - atan2(magY, magX).

4) Two small changes to the original code, both on the edges:
   - The old code could give 360 instead of 0 (same direction).
     fmodf keeps the result in 0 <= headingDeg < 360.
   - asin of a value just above 1 (sensor noise) gives NaN.
     clampToUnitRange cuts it to 1 first. Same idea as the acos clamp
     in azimuth().

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