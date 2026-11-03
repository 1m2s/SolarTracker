/**
 * @file MatlabLink.cpp
 * @brief Implementation of the USB line reader and the angle parser
 *
 * The numbers are read with strtod, not sscanf("%f"): on the Nano, sscanf
 * cannot read floats by default. The results are written only after every
 * check passed, so a broken line never changes the old angles.
 *
 * @author Mohamed Sellami
 * @date 2026-11-03
 *
 */
#include <stdlib.h>
#include "MatlabLink.h"

/******************************************************************************
 Constants
******************************************************************************/
/* Possible angles, deg. Anything outside is broken data. */
static const float AZIMUTH_MIN_DEG   = 0.0f;
static const float AZIMUTH_MAX_DEG   = 360.0f;
static const float ELEVATION_MIN_DEG = -90.0f;
static const float ELEVATION_MAX_DEG = 90.0f;

/******************************************************************************
 Private Function Declarations
******************************************************************************/
/**
 * @brief Check if a character is a blank
 *
 * @param[in] character  Character to check.
 *
 * @return true for space, tab, '\r' and '\n', false otherwise.
 */
static bool isBlank(char character);

/******************************************************************************
 Function Definitions
******************************************************************************/
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

    /* B3: never write past buf; remember the overflow and wait for '\n'. */
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
    /* A blank must follow, or "245.413.86" would be read as 245.413 and 0.86. */
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

    /* Written as !(in range) so that NaN is rejected too. */
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