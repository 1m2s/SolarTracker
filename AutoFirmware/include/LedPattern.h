#ifndef LEDPATTERN_H
#define LEDPATTERN_H

#include <stdint.h>
#include "config/Settings.h"

// Says if the LED should be on right now.
// nowMs = time from millis(). error = the last cycle had a problem.
bool ledOn(Mode mode, bool error, uint32_t nowMs);

#endif