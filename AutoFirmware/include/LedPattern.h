#ifndef LEDPATTERN_H
#define LEDPATTERN_H

#include <stdint.h>
#include "Mode.h"

// Says if the LED should be on right now.
// ms = time from millis(). error = the last cycle had a problem.
bool ledOn(Mode mode, bool error, uint32_t ms);

#endif