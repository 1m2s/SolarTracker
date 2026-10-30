#ifndef ISDUE_H
#define ISDUE_H

#include <stdint.h>

// True when at least "interval" ms have passed since "last".
// now and last come from millis(). Works across the millis() wrap.
bool isDue(uint32_t now, uint32_t last, uint32_t interval);

#endif