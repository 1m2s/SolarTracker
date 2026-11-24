#include "Display.h"

const int LAST_DIGIT = 7;
const int BRIGHTNESS = 15;

void showVoltage(LedControl &lc, int U)
{
    if (U < -999 || U > 999) {
        return;
    }
    if (U < 0) {
        U = -U;
    }
    int ones = U % 10;
    int tens = (U / 10) % 10;

    lc.setLed(0, 7, 2, true);
    lc.setLed(0, 7, 3, true);
    lc.setLed(0, 7, 4, true);
    lc.setLed(0, 7, 5, true);
    lc.setLed(0, 7, 6, true);
    lc.setLed(0, 6, 4, true);
    lc.setLed(0, 6, 7, true);

    lc.setDigit(0, 5, (byte)tens, false);
    lc.setDigit(0, 4, (byte)ones, false);
}

void showPower(LedControl &lc, int P)
{
    if (P < -999 || P > 999) {
        return;
    }
    if (P < 0) {
        P = -P;
    }
    int ones = P % 10;
    int tens = (P / 10) % 10;

    lc.setChar(0, 3, 'P', false);
    lc.setLed(0, 2, 4, true);
    lc.setLed(0, 2, 7, true);

    lc.setDigit(0, 1, (byte)tens, false);
    lc.setDigit(0, 0, (byte)ones, false);
}

void wakeDisplay(LedControl &lc)
{
    lc.setScanLimit(0, LAST_DIGIT);
    lc.setIntensity(0, BRIGHTNESS);
    lc.shutdown(0, false);
}