#ifndef MEASURE_H
#define MEASURE_H

// Turns the two analog readings (0..1023) into the panel's
// voltage Up (V) and power Pp (W).
// raw0 = A0 (voltage divider), raw1 = A1 (1/3 ohm shunt).
void measure(int raw0, int raw1, float &Up, float &Pp);

#endif