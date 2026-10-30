#ifndef MODE_H
#define MODE_H

#include <stdint.h>

// Who decides where the panel points.
// The switch on D2 picks the mode.
enum Mode : uint8_t {
    MODE_AUTO   = 0,    // switch open:   the Nano computes the sun position
    MODE_MATLAB = 1     // switch to GND: MATLAB sends the angles
};

#endif