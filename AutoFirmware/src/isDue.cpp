#include "isDue.h"

bool isDue(uint32_t now, uint32_t last, uint32_t interval)
{
    uint32_t passed = now - last;    // time since "last", also across the wrap
    return passed >= interval;
}