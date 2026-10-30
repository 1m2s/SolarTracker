#ifndef LINEREADER_H
#define LINEREADER_H

#include <stdint.h>

// Longest line we accept (characters, without the end marker)
const uint8_t LINE_LEN_MAX = 31;

// What lineFeed says after each character
enum LineStatus : uint8_t {
    LINE_NONE,        // line not finished yet (or an empty line)
    LINE_READY,       // a full line is now in r.buf
    LINE_TOO_LONG     // the line was too long and was thrown away
};

// Collects characters until '\n'. Call lineClear once before the first use.
struct LineReader {
    char    buf[LINE_LEN_MAX + 1];   // the line + end marker '\0'
    uint8_t len;                     // characters so far
    bool    tooLong;                 // more than LINE_LEN_MAX characters came
};

// Feed one character. Gives back what happened.
LineStatus lineFeed(LineReader &r, char c);

// Throw away a half line and start fresh.
void lineClear(LineReader &r);

#endif