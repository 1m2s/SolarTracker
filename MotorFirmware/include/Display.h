#ifndef DISPLAY_H
#define DISPLAY_H

#include <LedControl.h>

// 8-digit 7-segment display: "U=xx" on the left, "P=xx" on the right.
// Shows 0..99. Values outside -999..999 are not shown.
void showVoltage(LedControl &lc, int U);
void showPower(LedControl &lc, int P);

#endif