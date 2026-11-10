#ifndef CORRECTION_H
#define CORRECTION_H

float panelHeading(float sensorHeadingDeg);

float correctedAzimuth(float sunAzimuthDeg, float panelHeadingDeg);

float edgeRule(float lastSentDeg, float newDeg);

bool needsResend(float lastSentDeg, float newDeg);

#endif