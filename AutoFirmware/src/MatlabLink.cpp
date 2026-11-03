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
static const float ALPHA_MIN = 0.0f;
static const float ALPHA_MAX = 360.0f;
static const float GAMMA_MIN = -90.0f;
static const float GAMMA_MAX = 90.0f;

/******************************************************************************
 Private Function Declarations
******************************************************************************/
/**
 * @brief Check if a character is a blank
 *
 * @param[in] c  Character to check.
 *
 * @return true for space, tab, '\r' and '\n', false otherwise.
 */
static bool isBlank(char c);

/******************************************************************************
 Function Definitions
******************************************************************************/
LineStatus lineFeed(LineReader &r, char c)
{
    if (c == '\r') {
        return LINE_NONE;
    }

    if (c == '\n') {
        if (r.tooLong) {
            lineClear(r);
            return LINE_TOO_LONG;
        }
        if (r.len == 0) {
            return LINE_NONE;
        }
        r.buf[r.len] = '\0';
        r.len = 0;
        return LINE_READY;
    }

    /* B3: never write past buf; remember the overflow and wait for '\n'. */
    if (r.len < LINE_LEN_MAX) {
        r.buf[r.len] = c;
        r.len++;
    } else {
        r.tooLong = true;
    }
    return LINE_NONE;
}

void lineClear(LineReader &r)
{
    r.len     = 0;
    r.tooLong = false;
    r.buf[0]  = '\0';
}

static bool isBlank(char c)
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

bool parseAngles(const char *line, float &alpha, float &gamma)
{
    char *end;

    float a = (float)strtod(line, &end);
    if (end == line) {
        return false;
    }
    /* A blank must follow, or "245.413.86" would be read as 245.413 and 0.86. */
    if (!isBlank(*end)) {
        return false;
    }

    const char *p = end;
    float g = (float)strtod(p, &end);
    if (end == p) {
        return false;
    }

    while (isBlank(*end)) {
        end++;
    }
    if (*end != '\0') {
        return false;
    }

    /* Written as !(in range) so that NaN is rejected too. */
    if (!(a >= ALPHA_MIN && a <= ALPHA_MAX)) {
        return false;
    }
    if (!(g >= GAMMA_MIN && g <= GAMMA_MAX)) {
        return false;
    }

    alpha = a;
    gamma = g;
    return true;
}