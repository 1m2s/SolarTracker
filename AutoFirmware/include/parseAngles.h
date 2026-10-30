#ifndef PARSEANGLES_H
#define PARSEANGLES_H

// Reads a line from MATLAB like "245.41 3.86" (alpha gamma, in degrees).
// Line OK:     gives back true and fills alpha and gamma.
// Line broken: gives back false and does not change alpha and gamma.
bool parseAngles(const char *line, float &alpha, float &gamma);

#endif