#include "Measure.h"

void measure(int raw0, int raw1, float &Up, float &Pp)
{
    float U1 = raw0 * (5.0f / 1023.0f);   // A0 in volts
    float U2 = raw1 * (5.0f / 1023.0f);   // A1 in volts

    Up = (1 + 10000 / 2200) * U1;         // kept 1:1, see note 2
    Pp = Up * (U2 * 3);                   // I = U2 / (1/3 ohm) = U2 * 3
}

/* Notes
1) Same as readVoltages() in Photovoltaiknachfuehrung.ino.

2) Known bug, kept on purpose for now (1:1 port):
   10000 / 2200 is integer division -> 4, not 4.545.
   So the factor is 5 instead of 5.545 and U and P read about 10 % low.
   Divider in the Schaltplan: 10 k (plus side), 2.2 k (ground side).
   test_measure shows it. Fixing it is a separate decision.

3) Power: three 1 ohm resistors in parallel = 1/3 ohm.
   I = U2 / (1/3) = 3 * U2, and P = Up * I.
*/