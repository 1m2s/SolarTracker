/* Notes
   AUTO mode: the Nano computes the sun position itself.
   This is AusgabeSonnenstand.m in C++.
   Every 30 s: read clock -> SonnenStand -> /2 and uint8 -> send + print.
   SEND_I2C = true: values go to the right Nano, the motors move.
   The clock is always MEZ (winter time), so tf = 0.
   Bad clock time -> nothing is computed, nothing is sent.
   ack = answer of the right Nano: 0 = OK, 2 = no answer.
*/

#include <Arduino.h>
#include <Wire.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>
#include "SonnenStand.h"
#include "toUint8.h"

// ---------- settings ----------
const bool     SEND_I2C    = true;    // NEW: false = Serial only, true = motors move
const uint8_t  SLAVE_ADDR  = 0x19;    // right Nano (A_AIDE)
const uint32_t INTERVAL_MS = 30000;   // MATLAB: pause(30)

// Standort (same as MATLAB)
const float Breite = 47.81f;
const float Laenge = 9.66f;

// ---------- clock ----------
const uint8_t PIN_CLK = 5;
const uint8_t PIN_DAT = 6;
const uint8_t PIN_RST = 7;

ThreeWire rtcWire(PIN_DAT, PIN_CLK, PIN_RST);   // order: IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(rtcWire);

// MATLAB: writeRegister(slave, reg, value, 'uint8')
// Gives back the answer code: 0 = OK (ACK), 2 = no answer   // NEW
uint8_t writeRegister(uint8_t reg, uint8_t value)            // NEW: uint8_t, not void
{
    Wire.beginTransmission(SLAVE_ADDR);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission();                           // NEW: return the code
}

void setup()
{
    Serial.begin(115200);
    Rtc.Begin();                       // only sets pin modes, writes nothing
    if (SEND_I2C) {
        Wire.begin();                  // Nano is I2C master, 100 kHz like MATLAB
    }
    Serial.print(F("AUTO mode, SEND_I2C = "));
    Serial.println(SEND_I2C ? F("ON") : F("OFF"));
}

void loop()
{
    RtcDateTime t = Rtc.GetDateTime();   // MATLAB: clock

    // Clock check: same rule as test_rtc (valid date, year 2026 or later)
    if (!t.IsValid() || t.Year() < 2026) {
        Serial.println(F("RTC not valid -> nothing sent"));
        delay(INTERVAL_MS);
        return;
    }

    float a, g;
    SonnenStand(Breite, Laenge, t.Day(), t.Month(), t.Hour(), t.Minute(), 0, a, g);

    uint8_t a8 = toUint8(a / 2.0f);      // slave doubles it again
    uint8_t g8 = toUint8(g);

    char buf[20];
    snprintf(buf, sizeof(buf), "%04u-%02u-%02u %02u:%02u:%02u",
             t.Year(), t.Month(), t.Day(), t.Hour(), t.Minute(), t.Second());
    Serial.print(buf);
    Serial.print(F("  a="));  Serial.print(a, 4);
    Serial.print(F("  g="));  Serial.print(g, 4);
    Serial.print(F("  a8=")); Serial.print(a8);
    Serial.print(F("  g8=")); Serial.print(g8);              // NEW: print, not println

    if (SEND_I2C) {
        uint8_t ackA = writeRegister(254, a8);               // NEW: keep the answer
        uint8_t ackG = writeRegister(255, g8);               // NEW
        Serial.print(F("  ack="));                           // NEW
        Serial.print(ackA);                                  // NEW
        Serial.print('/');                                   // NEW
        Serial.print(ackG);                                  // NEW
    }
    Serial.println();                                        // NEW: end the line

    delay(INTERVAL_MS);
}