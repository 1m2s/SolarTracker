#include <Arduino.h>
#include "grad2rad.h"
#include "rad2grad.h"

void setup() {
    Serial.begin(115200);
    Serial.println(rad2grad(1.0f), 7);             // ~57.29578
    Serial.println(rad2grad(grad2rad(45.0f)), 7);  // ~45.00000
}

void loop() {}