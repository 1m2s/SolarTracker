#include <stdlib.h>
#include "MatlabLink.h"

static const float AZIMUTH_MIN_DEG   = 0.0f;
static const float AZIMUTH_MAX_DEG   = 360.0f;
static const float ELEVATION_MIN_DEG = -90.0f;
static const float ELEVATION_MAX_DEG = 90.0f;

static bool isBlank(char character);

LineStatus lineFeed(LineReader &reader, char character)
{
    if (character == '\r') {
        return LINE_NONE;
    }

    if (character == '\n') {
        if (reader.tooLong) {
            lineClear(reader);
            return LINE_TOO_LONG;
        }
        if (reader.len == 0) {
            return LINE_NONE;
        }
        reader.buf[reader.len] = '\0';
        reader.len = 0;
        return LINE_READY;
    }

    if (reader.len < LINE_LEN_MAX) {
        reader.buf[reader.len] = character;
        reader.len++;
    } else {
        reader.tooLong = true;
    }
    return LINE_NONE;
}

void lineClear(LineReader &reader)
{
    reader.len     = 0;
    reader.tooLong = false;
    reader.buf[0]  = '\0';
}

static bool isBlank(char character)
{
    return character == ' ' || character == '\t' || character == '\r' || character == '\n';
}

bool parseAngles(const char *line, float &azimuthDeg, float &elevationDeg)
{
    char *parseEnd;

    float parsedAzimuthDeg = (float)strtod(line, &parseEnd);
    if (parseEnd == line) {
        return false;
    }
    if (!isBlank(*parseEnd)) {
        return false;
    }

    const char *secondStart = parseEnd;
    float parsedElevationDeg = (float)strtod(secondStart, &parseEnd);
    if (parseEnd == secondStart) {
        return false;
    }

    while (isBlank(*parseEnd)) {
        parseEnd++;
    }
    if (*parseEnd != '\0') {
        return false;
    }

    if (!(parsedAzimuthDeg >= AZIMUTH_MIN_DEG && parsedAzimuthDeg <= AZIMUTH_MAX_DEG)) {
        return false;
    }
    if (!(parsedElevationDeg >= ELEVATION_MIN_DEG && parsedElevationDeg <= ELEVATION_MAX_DEG)) {
        return false;
    }

    azimuthDeg   = parsedAzimuthDeg;
    elevationDeg = parsedElevationDeg;
    return true;
}