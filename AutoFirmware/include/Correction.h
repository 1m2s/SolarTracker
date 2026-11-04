#ifndef CORRECTION_H
#define CORRECTION_H

float arrowHeading(float sensorHeadingDeg);

float correctedAzimuth(float sunAzimuthDeg, float arrowHeadingDeg);

float edgeRule(float lastSentDeg, float newDeg);

bool needsResend(float lastSentDeg, float newDeg);

#endif