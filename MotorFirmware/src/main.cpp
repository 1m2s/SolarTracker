/* Notes
   Right Nano (A_AIDE): moves the panel and shows U and P.
   I2C slave 0x19. The left Nano (or MATLAB) sends two messages:
     [254, a8]   azimuth  = a8 * 2 deg
     [255, g8]   elevation = g8 deg
   Behaves like mode "ohne IMU" of Photovoltaiknachfuehrung.ino:
   at power-on the panel must face south and stand vertical.
   No IMU and no IMU switch here: the left Nano decides everything.
*/

#include <Arduino.h>
#include <Wire.h>
#include <Stepper.h>
#include <Servo.h>
#include <LedControl.h>
#include "Motion.h"
#include "Measure.h"
#include "Display.h"

// ---------- settings (same values as the thesis) ----------
const uint8_t SLAVE_ADDR = 0x19;    // 25, the address MATLAB and the left Nano use
const float   SNull      = 95;      // servo angle for 0 deg elevation
const float   SKorr      = 1.2f;    // correction factor for the servo
const float   STEP_DEG   = 1.8f;    // 200 steps per turn

// ---------- hardware ----------
Stepper    stepper_1(200, 5, 4, 3, 2);    // L298N on D5, D4, D3, D2
Servo      servo_2;                       // signal on D9
LedControl lc = LedControl(10, 12, 11, 1);   // DIN D10, CLK D12, CS D11

// ---------- written by the I2C interrupt ----------
volatile uint8_t a8 = 90;   // 90 * 2 = 180 deg = south (start position)
volatile uint8_t g8 = 0;    // 0 deg = vertical (start position)

// ---------- state ----------
float off1 = 0;   // how far the stepper turned since power-on (deg)
float off2 = 0;   // current servo elevation (deg)

// Runs inside the I2C interrupt when a message arrives.
// Only full 2-byte messages [register, value] are used.
void onMessage(int howMany)
{
    if (howMany != 2) {
        while (Wire.available() > 0) {
            Wire.read();               // throw away broken message
        }
        return;
    }
    uint8_t reg = Wire.read();
    uint8_t val = Wire.read();
    if (reg == 254) {
        a8 = val;
    } else if (reg == 255) {
        g8 = val;
    }
}

void setup()
{
    Wire.begin(SLAVE_ADDR);            // join the bus as slave 0x19
    Wire.setClock(100000);             // 100 kHz, same as before
    Wire.onReceive(onMessage);
    Serial.begin(115200);

    stepper_1.setSpeed(6);             // 6 turns per minute
    servo_2.attach(9);

    lc.shutdown(0, false);             // wake the display up
    lc.setIntensity(0, 15);            // brightness, 15 = max
    lc.clearDisplay(0);

    Serial.println(F("MotorFirmware (ohne IMU)"));
}

void loop()
{
    float azi = 2.0f * a8;             // one byte each: safe to read here
    float gho = g8;

    // 1) Stepper: one step toward the target
    int8_t dir = stepDirection(azi, off1);
    if (dir != 0) {
        stepper_1.step(dir);
        off1 += dir * STEP_DEG;
    }
    delay(60);                         // keeps the old speed (see note 3)

    // 2) Servo: one degree toward the target
    off2 = servoStep(gho, off2);
    servo_2.write((int)(SNull + off2 * SKorr));

    // 3) Measure and show U and P
    float Up, Pp;
    measure(analogRead(A0), analogRead(A1), Up, Pp);
    showVoltage(lc, round(Up));
    showPower(lc, round(Pp));
}

/* Notes
1) What changed compared to Photovoltaiknachfuehrung.ino:
   - No IMU, no BMP280, no IMU switch (D7 is not read any more).
   - Messages are read ONLY in onMessage (the interrupt), not also
     in loop(). Before, both read the same buffer.
   - onMessage has the right form void(int) and checks the length.
   - a8 and g8 are stored as bytes. A byte is copied in one go,
     so the interrupt can never change it halfway through.
     (A float is 4 bytes and could be torn.)

2) What stayed the same: address, registers 254/255, start position,
   step rule, servo rule, speed, display. Motion.cpp, Measure.cpp and
   Display.cpp have their own notes.

3) delay(60) was in setStepper_2() before. One loop takes about
   60 ms plus the step, so the servo moves about 1 deg per 60 ms.

4) The (int) before SNull + off2 * SKorr: Servo.write() takes whole
   degrees. The old code cut off the decimals too, just without
   showing it.
*/