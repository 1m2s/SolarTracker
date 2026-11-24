#ifndef DISPLAY_H
#define DISPLAY_H

#include <LedControl.h>

void showVoltage(LedControl &lc, int U);
void showPower(LedControl &lc, int P);
void wakeDisplay(LedControl &lc);

#endif