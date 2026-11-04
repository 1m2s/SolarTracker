#include "Heading.h"
#include "config/Calibration.h"
#include <math.h>

const float MAG_SCALE = 1200.0f / 4096.0f;

const float DEG_PER_RAD = 180.0f / (float)M_PI;

void magCalibrated(int16_t magRawX, int16_t magRawY, int16_t magRawZ, float &magX, float &magY, float &magZ)
{
    float centredX = magRawX * MAG_SCALE - MX_CENTRE;
    float centredY = magRawY * MAG_SCALE - MY_CENTRE;

    magX = SOFT_XX * centredX + SOFT_XY * centredY;
    magY = SOFT_XY * centredX + SOFT_YY * centredY;

    magZ = magRawZ * MAG_SCALE - MZ_CENTRE;
}

static float clampToUnitRange(float value)
{
    if (value > 1.0f) {
        return 1.0f;
    }
    if (value < -1.0f) {
        return -1.0f;
    }
    if (!(value >= -1.0f)) {
        return 0.0f;
    }
    return value;
}

float tiltHeading(float accelX, float accelY, float magX, float magY, float magZ)
{
    float pitchRad = asinf(clampToUnitRange(-accelX));
    float rollRad  = asinf(clampToUnitRange(accelY / cosf(pitchRad)));

    float horizontalX = magX * cosf(pitchRad) + magZ * sinf(pitchRad);
    float horizontalY = magX * sinf(rollRad) * sinf(pitchRad) + magY * cosf(rollRad)
                      - magZ * sinf(rollRad) * cosf(pitchRad);

    float headingDeg = atan2f(horizontalY, horizontalX) * DEG_PER_RAD;
    if (horizontalY < 0) {
        headingDeg += 360.0f;
    }
    headingDeg = 360.0f - headingDeg;

    headingDeg = fmodf(headingDeg, 360.0f);
    if (headingDeg < 0) {
        headingDeg += 360.0f;
    }
    return headingDeg;
}