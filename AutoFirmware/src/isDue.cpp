#include "isDue.h"

bool isDue(uint32_t now, uint32_t last, uint32_t interval)
{
    uint32_t passed = now - last;    // time since "last", also across the wrap
    return passed >= interval;
}

/* Notes
1) Like a kitchen timer: "has it been 30 s since I last looked?"
 
2) millis() counts up to 4294967295, then starts again at 0.
   That is after about 49.7 days.
 
3) now - last still gives the right time after the wrap.
   Example: last = 4294967290 (6 ms before the wrap), now = 4.
   Real time passed: 6 + 4 = 10 ms. And now - last = 10. Correct.
   Reason: uint32_t math always wraps around the same way.
 
4) The wrong way is: now >= last + interval.
   last + interval can wrap to a small number.
   Then the check is true much too early.
   test_isDue shows this.
*/