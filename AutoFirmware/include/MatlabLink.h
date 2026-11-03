#ifndef MATLABLINK_H
#define MATLABLINK_H

#include <stdint.h>

const uint8_t LINE_LEN_MAX = 31;

enum LineStatus : uint8_t {
    LINE_NONE,
    LINE_READY,
    LINE_TOO_LONG
};

struct LineReader {
    char    buf[LINE_LEN_MAX + 1];
    uint8_t len;
    bool    tooLong;
};

LineStatus lineFeed(LineReader &reader, char character);

void lineClear(LineReader &reader);

bool parseAngles(const char *line, float &azimuthDeg, float &elevationDeg);

#endif