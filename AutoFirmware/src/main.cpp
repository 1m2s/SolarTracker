#include <Arduino.h>
#include "grad2rad.h"
#include "rad2grad.h"
#include "JahresTag.h"
#include "MEZ.h"
#include "MOZ.h"

void setup() {
    Serial.begin(115200);

    // grad2rad
    Serial.println(grad2rad(0.0f), 7);             // 0.0000000
    Serial.println(grad2rad(45.0f), 7);            // 0.7853982
    Serial.println(grad2rad(90.0f), 7);            // 1.5707963
    Serial.println(grad2rad(180.0f), 7);           // 3.1415927

    // rad2grad
    Serial.println(rad2grad(1.0f), 7);             // 57.2957763
    Serial.println(rad2grad(grad2rad(45.0f)), 7);  // 45.0000000

    // JahresTag
    Serial.println(JahresTag(1, 1));               // 1
    Serial.println(JahresTag(3, 1));               // 60
    Serial.println(JahresTag(10, 26));             // 299
    Serial.println(JahresTag(12, 31));             // 365

    // MEZ
    Serial.println(MEZ(12, 0, 0));                 // 720
    Serial.println(MEZ(13, 0, 1));                 // 720
    Serial.println(MEZ(15, 30, 0));                // 930
    Serial.println(MEZ(0, 30, 1));                 // -30

    // MOZ
    Serial.println(MOZ(720, 15.0f), 4);            // 720.0000  (on the 15 deg line: no shift)
    Serial.println(MOZ(720, 9.66f), 4);            // 698.6400
    Serial.println(MOZ(930, 9.66f), 4);            // 908.6400
    Serial.println(MOZ(MEZ(12, 0, 0), 9.66f), 4);  // 698.6400  (MEZ -> MOZ chained)
}

void loop() {}