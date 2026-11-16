#include <Arduino.h>
#include <Wire.h>
#include <unity.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "Imu.h"
#include "Heading.h"

const uint32_t SETTLE_MS  = 2000uL;
const uint32_t MEASURE_MS = 20000uL;
const uint32_t PRINT_MS   = 1000uL;

const float ACCEL_PER_G = 16384.0f;

void setUp(void) {}
void tearDown(void) {}

static void printDegrees(const char *label, float degrees)
{
    char line[64];
    long tenths = lround(degrees * 10.0f);
    const char *sign = (tenths < 0L) ? "-" : "";
    unsigned long magnitude = (unsigned long)labs(tenths);

    snprintf(line, sizeof(line), "%s%s%lu.%lu", label, sign, magnitude / 10uL, magnitude % 10uL);
    TEST_MESSAGE(line);
}

static float headingOf(int16_t accelX, int16_t accelY, int16_t magRawX, int16_t magRawY, int16_t magRawZ,
                       float &fieldXY)
{
    float magX = 0.0f;
    float magY = 0.0f;
    float magZ = 0.0f;

    magCalibrated(magRawX, magRawY, magRawZ, magX, magY, magZ);
    fieldXY = sqrtf((magX * magX) + (magY * magY));
    return tiltHeading((float)accelX / ACCEL_PER_G, (float)accelY / ACCEL_PER_G, magX, magY, magZ);
}

void test_wake_up(void)
{
    TEST_ASSERT_TRUE_MESSAGE(imuBegin(), "MPU-9250 or compass (0x0C) does not answer");
}

void test_average_heading(void)
{
    int32_t  sumAccelX = 0;
    int32_t  sumAccelY = 0;
    int32_t  sumMagX   = 0;
    int32_t  sumMagY   = 0;
    int32_t  sumMagZ   = 0;
    uint16_t goodReads = 0u;
    uint32_t badReads  = 0uL;
    float    fieldXY   = 0.0f;
    char     line[64];

    TEST_MESSAGE("Hands off, laptop and phone at least 1 m away. Measuring starts in 2 s");
    delay(SETTLE_MS);

    uint32_t start     = millis();
    uint32_t lastPrint = start;

    while ((millis() - start) < MEASURE_MS) {
        ImuRaw raw = {0, 0, 0, 0, 0};
        if (!imuRead(raw)) {
            badReads++;
            delay(10uL);
            continue;
        }
        goodReads++;
        sumAccelX += raw.accelX;
        sumAccelY += raw.accelY;
        sumMagX   += raw.magX;
        sumMagY   += raw.magY;
        sumMagZ   += raw.magZ;

        if ((millis() - lastPrint) >= PRINT_MS) {
            lastPrint += PRINT_MS;
            printDegrees("h now = ", headingOf(raw.accelX, raw.accelY, raw.magX, raw.magY, raw.magZ, fieldXY));
        }
    }

    snprintf(line, sizeof(line), "good reads: %u, bad reads: %lu", (unsigned)goodReads, (unsigned long)badReads);
    TEST_MESSAGE(line);
    TEST_ASSERT_TRUE_MESSAGE(goodReads > 0u, "no good read: compass dead or unplugged");

    int16_t avgAccelX = (int16_t)(sumAccelX / (int32_t)goodReads);
    int16_t avgAccelY = (int16_t)(sumAccelY / (int32_t)goodReads);
    int16_t avgMagX   = (int16_t)(sumMagX / (int32_t)goodReads);
    int16_t avgMagY   = (int16_t)(sumMagY / (int32_t)goodReads);
    int16_t avgMagZ   = (int16_t)(sumMagZ / (int32_t)goodReads);

    float sensorHeading = headingOf(avgAccelX, avgAccelY, avgMagX, avgMagY, avgMagZ, fieldXY);

    snprintf(line, sizeof(line), "average raw m=%d %d %d  a=%d %d", avgMagX, avgMagY, avgMagZ, avgAccelX, avgAccelY);
    TEST_MESSAGE(line);
    printDegrees("field strength xy = ", fieldXY);
    printDegrees("AVERAGE h_sensor = ", sensorHeading);
}

void setup()
{
    delay(2000);
    Wire.begin();
    Wire.setWireTimeout(3000, true);
    UNITY_BEGIN();
    RUN_TEST(test_wake_up);
    RUN_TEST(test_average_heading);
    UNITY_END();
}

void loop() {}