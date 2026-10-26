#include <Arduino.h>
#include "grad2rad.h"

void setup()
{
    Serial.begin(115200);
    delay(1000);   // give the Serial Monitor time to connect

    // Check type sizes (for the note in grad2rad.cpp)
    Serial.print("sizeof(float)  = ");
    Serial.println(sizeof(float));
    Serial.print("sizeof(double) = ");
    Serial.println(sizeof(double));

    // Test grad2rad against MATLAB values
    Serial.print("grad2rad(0)    = ");
    Serial.println(grad2rad(0.0f), 7);     // MATLAB: 0
    Serial.print("grad2rad(45)   = ");
    Serial.println(grad2rad(45.0f), 7);    // MATLAB: 0.785398163397448
    Serial.print("grad2rad(90)   = ");
    Serial.println(grad2rad(90.0f), 7);    // MATLAB: 1.570796326794897
    Serial.print("grad2rad(180)  = ");
    Serial.println(grad2rad(180.0f), 7);   // MATLAB: 3.141592653589793
}

void loop()
{
}