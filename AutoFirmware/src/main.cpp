/* Notes
   AUTO mode: the Nano computes the sun position itself.
   This is AusgabeSonnenstand.m in C++.
   Every 30 s: read clock -> SonnenStand -> /2 and uint8 -> print.
   SEND_I2C = false: nothing goes to the motors yet. Only Serial.
   The clock is always MEZ (winter time), so tf = 0.
   Bad clock time -> nothing is computed, nothing is sent.
*/

#include <Arduino.h>
#include <Wire.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>
#include "SonnenStand.h"
#include "toUint8.h"

// ---------- settings ----------
const bool     SEND_I2C    = false;   // true = send to the motors (step 2.5)
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
void writeRegister(uint8_t reg, uint8_t value)
{
    Wire.beginTransmission(SLAVE_ADDR);
    Wire.write(reg);
    Wire.write(value);
    Wire.endTransmission();
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
    Serial.print(F("  g8=")); Serial.println(g8);

    if (SEND_I2C) {
        writeRegister(254, a8);
        writeRegister(255, g8);
    }

    delay(INTERVAL_MS);
}