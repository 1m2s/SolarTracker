#include "Motion.h"

const float START_AZI = 180.0f;   // panel faces south at power-on
const float HYS       = 0.9f;     // half a step (1.8 / 2)

int8_t stepDirection(float azi, float off1)
{
    float panel = START_AZI + off1;   // where the panel points now
    if (azi > panel + HYS) {
        return 1;
    }
    if (azi < panel - HYS) {
        return -1;
    }
    return 0;
}

float servoStep(float gho, float off2)
{
    if (gho > off2) {
        return off2 + 1.0f;
    }
    if (gho < off2) {
        return off2 - 1.0f;
    }
    return off2;
}

/* Notes
1) Same rules as setStepper_2() and setServo_2() in
   Photovoltaiknachfuehrung.ino. Only the motor calls moved to main.cpp,
   so these two functions can be tested on the PC.

2) 200 steps per turn -> one step = 360 / 200 = 1.8 deg.
   HYS = 0.9 = half a step. So the panel stops within
   0.9 deg of the target and never jumps back and forth.

3) Example: target 258 deg, start 180 deg.
   It needs 78 deg -> 43 steps (43 * 1.8 = 77.4, then
   258 - 257.4 = 0.6 < 0.9 -> stop). test_motion checks this.

4) The servo moves only 1 deg per loop, so it turns slowly.
*/