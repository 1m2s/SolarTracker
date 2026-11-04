#include "Display.h"

void showVoltage(LedControl &lc, int U)
{
    if (U < -999 || U > 999) {
        return;
    }
    if (U < 0) {
        U = -U;                    // sign is not shown
    }
    int ones = U % 10;
    int tens = (U / 10) % 10;

    // "U=" built from single segments (digit 7 and 6)
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

    // "P=" (digit 3 and 2)
    lc.setChar(0, 3, 'P', false);
    lc.setLed(0, 2, 4, true);
    lc.setLed(0, 2, 7, true);

    lc.setDigit(0, 1, (byte)tens, false);
    lc.setDigit(0, 0, (byte)ones, false);
}

/* Notes
1) Same as printVoltage() and printPower() in Photovoltaiknachfuehrung.ino.
   Unused variables (hundreds, negative) were removed. What the display
   shows did not change.

2) Digits count from the right: 0 = rightmost, 7 = leftmost.

3) Only tens and ones are shown, so 0..99. Hundreds are cut off,
   e.g. 123 shows as "23". Same as before.
*/