#include "LineReader.h"

LineStatus lineFeed(LineReader &r, char c)
{
    if (c == '\r') {
        return LINE_NONE;                // skip, '\n' ends the line
    }

    if (c == '\n') {                     // end of the line
        if (r.tooLong) {
            lineClear(r);                // throw the long line away
            return LINE_TOO_LONG;
        }
        if (r.len == 0) {
            return LINE_NONE;            // empty line: ignore it
        }
        r.buf[r.len] = '\0';             // end marker -> now a normal C text
        r.len = 0;                       // next character starts a new line
        return LINE_READY;
    }

    if (r.len < LINE_LEN_MAX) {
        r.buf[r.len] = c;                // store the character
        r.len++;
    } else {
        r.tooLong = true;                // no room: remember it, wait for '\n'
    }
    return LINE_NONE;
}

void lineClear(LineReader &r)
{
    r.len     = 0;
    r.tooLong = false;
    r.buf[0]  = '\0';
}

/* Notes
1) USB data comes one character at a time.
   LineReader collects them, like writing a word letter by letter.
   Only at '\n' is the line finished.
 
2) '\r' is skipped. Some programs send "\r\n" at the end.
   So "245.41 3.86\r\n" becomes "245.41 3.86".
 
3) Too long: we stop storing, but keep reading until '\n'.
   Then we say LINE_TOO_LONG and start fresh.
   So a long line never runs over the memory,
   and never sticks to the next line.
 
4) Empty line (only Enter): LINE_NONE. Nothing to do.
 
5) After LINE_READY, use r.buf right away.
   The next character starts writing over it.
 
6) Why not LINE_MAX as the name?
   Linux already uses LINE_MAX. Same name = clash on the PC and in CI.
*/