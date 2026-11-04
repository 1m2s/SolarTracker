#include "Correction.h"
#include "config/Calibration.h"
#include "config/Settings.h"
#include <math.h>

static const float FULL_TURN_DEG = 360.0f;
static const float HALF_TURN_DEG = 180.0f;

static float wrapDeg(float angleDeg)
{
    float wrappedDeg = fmodf(angleDeg, FULL_TURN_DEG);
    if (wrappedDeg < 0.0f) {
        wrappedDeg += FULL_TURN_DEG;
    }
    if (wrappedDeg >= FULL_TURN_DEG) {
        wrappedDeg = 0.0f;
    }
    return wrappedDeg;
}

float arrowHeading(float sensorHeadingDeg)
{
    return wrapDeg(sensorHeadingDeg + IMU_TO_ARROW_DEG + MAG_DECLINATION_DEG);
}

float correctedAzimuth(float sunAzimuthDeg, float arrowHeadingDeg)
{
    return wrapDeg(sunAzimuthDeg - arrowHeadingDeg + HALF_TURN_DEG);
}

float edgeRule(float lastSentDeg, float newDeg)
{
    bool lastNearTop      = lastSentDeg >= (FULL_TURN_DEG - EDGE_MARGIN_DEG);
    bool lastNearBottom   = lastSentDeg < EDGE_MARGIN_DEG;
    bool newJustPastZero  = newDeg < EDGE_MARGIN_DEG;
    bool newJustBelow360  = newDeg > (FULL_TURN_DEG - EDGE_MARGIN_DEG);

    if (lastNearTop && newJustPastZero) {
        return lastSentDeg;
    }
    if (lastNearBottom && newJustBelow360) {
        return lastSentDeg;
    }
    return newDeg;
}

bool needsResend(float lastSentDeg, float newDeg)
{
    float differenceDeg = fabsf(newDeg - lastSentDeg);
    return differenceDeg >= RESEND_DEG;
}