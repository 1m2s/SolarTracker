#include "parseAngles.h"
#include <stdlib.h>    // strtod

// Possible values in degrees. Outside = broken data.
const float ALPHA_MIN = 0.0f;      // azimuth: north
const float ALPHA_MAX = 360.0f;    // azimuth: north again
const float GAMMA_MIN = -90.0f;    // elevation: straight down
const float GAMMA_MAX = 90.0f;     // elevation: straight up

// true for space, tab, \r and \n
static bool isBlank(char c)
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

bool parseAngles(const char *line, float &alpha, float &gamma)
{
    char *end;                                // strtod puts here where it stopped

    // 1) First number
    float a = (float)strtod(line, &end);
    if (end == line) {
        return false;                         // no number at the start
    }
    if (!isBlank(*end)) {
        return false;                         // must be followed by a space
    }

    // 2) Second number
    const char *p = end;
    float g = (float)strtod(p, &end);
    if (end == p) {
        return false;                         // second number missing
    }

    // 3) Only spaces, \r or \n may follow
    while (isBlank(*end)) {
        end++;
    }
    if (*end != '\0') {
        return false;                         // something else is left -> garbage
    }

    // 4) Range check (also false for NaN)
    if (!(a >= ALPHA_MIN && a <= ALPHA_MAX)) {
        return false;
    }
    if (!(g >= GAMMA_MIN && g <= GAMMA_MAX)) {
        return false;
    }

    // 5) All OK -> only now write the results
    alpha = a;
    gamma = g;
    return true;
}

/* Notes
1) strtod = "string to double". It reads a number from text.
   It skips spaces at the start.
   It stops at the first sign that is not part of the number.
   It writes that stop place into "end".
 
2) end == line means: strtod could not read anything.
   Example: "hello". Then there is no number.
 
3) Why must a space follow the first number?
   Without this check, "245.413.86" would be read as
   245.413 and 0.86. That is wrong. So we demand a space.
 
4) Why sscanf("%f") is not used:
   On the Nano, sscanf cannot read floats by default.
   strtod can. The Nano test proves it.
 
5) alpha and gamma are written only at the end.
   So a broken line never changes the old values.
 
6) Night values like "250.1 -5.2" are OK here.
   targetBytes parks them later, same as in AUTO.
   Only impossible values (like 400 deg) are rejected.
 
7) !(a >= MIN && a <= MAX) is also true for NaN.
   Same trick as in targetBytes.
*/
