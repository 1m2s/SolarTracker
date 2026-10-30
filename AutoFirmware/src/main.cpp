/* Notes
   One firmware, two modes. The switch on D2 picks the mode.
   AUTO   (switch open):   the Nano computes the sun position itself.
                           This is AusgabeSonnenstand.m in C++.
                           Lines from USB are ignored.
   MATLAB (switch to GND): MATLAB sends "alpha gamma" lines over USB,
                           e.g. "245.41 3.86". The Nano parks at night
                           with targetBytes, same as in AUTO.
   No delay(): loop() runs many times per second.
   Each pass: read the switch, read USB, update the LED.
   AUTO: one cycle every 30 s (and right after switching to AUTO).
   MATLAB: no good line for 180 s -> timeout, the panel stays.
   LED on D13: fast blink = error, on = MATLAB, slow blink = AUTO OK.
   The clock is always MEZ (winter time), so tf = 0.
   ack = answer of the right Nano: 0 = OK, 2 = no answer.
*/

#include <Arduino.h>
#include <Wire.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>
#include "SonnenStand.h"
#include "targetBytes.h"
#include "Mode.h"
#include "LedPattern.h"
#include "isDue.h"
#include "LineReader.h"
#include "parseAngles.h"

// ---------- settings ----------
const bool     SEND_I2C          = true;      // false = Serial only, true = motors move
const uint8_t  SLAVE_ADDR        = 0x19;      // right Nano (A_AIDE)
const uint32_t INTERVAL_MS       = 30000UL;   // MATLAB: pause(30)
const uint32_t DEBOUNCE_MS       = 50;        // switch must be still this long
const uint32_t MATLAB_TIMEOUT_MS = 180000UL;  // 6 missed MATLAB lines (6 x 30 s)

// Standort (same as MATLAB)
const float Breite = 47.81f;
const float Laenge = 9.66f;

// ---------- pins ----------
const uint8_t PIN_MODE = 2;     // switch: open = AUTO, GND = MATLAB
const uint8_t PIN_LED  = 13;    // external LED + built-in LED

// ---------- clock ----------
const uint8_t PIN_CLK = 5;
const uint8_t PIN_DAT = 6;
const uint8_t PIN_RST = 7;

ThreeWire rtcWire(PIN_DAT, PIN_CLK, PIN_RST);   // order: IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(rtcWire);

// ---------- state (kept between loop passes) ----------
Mode       mode       = MODE_AUTO;   // mode we act on
bool       error      = false;       // something is wrong -> fast blink
bool       runNow     = true;        // true = do an AUTO cycle now, don't wait
uint32_t   lastCycle  = 0;           // millis() of the last AUTO cycle
bool       lastRaw    = HIGH;        // last raw reading of the switch pin
uint32_t   rawSince   = 0;           // millis() when that reading last changed
LineReader reader;                   // collects USB characters into a line
uint32_t   lastMatlab = 0;           // millis() of the last good MATLAB line
bool       timedOut   = false;       // MATLAB timeout already reported

// MATLAB: writeRegister(slave, reg, value, 'uint8')
// Gives back the answer code: 0 = OK (ACK), 2 = no answer
uint8_t writeRegister(uint8_t reg, uint8_t value)
{
    Wire.beginTransmission(SLAVE_ADDR);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission();
}

// Used by AUTO and MATLAB: angles -> bytes -> right Nano.
// Prints "  a=..  g=..  a8=..  g8=..  [PARK]  ack=../.." and ends the line.
// Gives back true if the right Nano answered OK (or SEND_I2C is off).
bool sendTarget(float a, float g)
{
    uint8_t a8, g8;
    bool parked = targetBytes(a, g, a8, g8);   // day: sun bytes, night: park bytes

    Serial.print(F("  a="));  Serial.print(a, 4);
    Serial.print(F("  g="));  Serial.print(g, 4);
    Serial.print(F("  a8=")); Serial.print(a8);
    Serial.print(F("  g8=")); Serial.print(g8);
    if (parked) {
        Serial.print(F("  PARK"));
    }

    bool ackOk = true;
    if (SEND_I2C) {
        uint8_t ackA = writeRegister(254, a8);
        uint8_t ackG = writeRegister(255, g8);
        Serial.print(F("  ack="));
        Serial.print(ackA);
        Serial.print('/');
        Serial.print(ackG);
        ackOk = (ackA == 0 && ackG == 0);
    }
    Serial.println();
    return ackOk;
}

void printMode()
{
    if (mode == MODE_AUTO) {
        Serial.println(F("Mode -> AUTO"));
    } else {
        Serial.println(F("Mode -> MATLAB (waiting for a line)"));
    }
}

// Throws away everything that came over USB but was not used yet.
void flushInput()
{
    while (Serial.available() > 0) {
        Serial.read();                   // USB buffer of the Nano
    }
    lineClear(reader);                   // half line in our LineReader
}

// Reads the switch. Gives back true when the mode has changed.
// A new position counts only after it was still for DEBOUNCE_MS.
bool updateMode(uint32_t now)
{
    bool raw = digitalRead(PIN_MODE);    // HIGH = open = AUTO
    if (raw != lastRaw) {                // pin moved (or bounced)
        lastRaw  = raw;
        rawSince = now;                  // start the "still" timer again
        return false;
    }
    Mode m = raw ? MODE_AUTO : MODE_MATLAB;
    if (m != mode && isDue(now, rawSince, DEBOUNCE_MS)) {
        mode = m;                        // still long enough -> accept
        return true;
    }
    return false;
}

// One AUTO cycle: read clock -> SonnenStand -> sendTarget.
// Sets error = true if the clock is bad or the right Nano does not answer.
void autoCycle()
{
    RtcDateTime t = Rtc.GetDateTime();   // MATLAB: clock

    // Clock check: same rule as test_rtc (valid date, year 2026 or later)
    if (!t.IsValid() || t.Year() < 2026) {
        Serial.println(F("AUTO    RTC not valid -> nothing sent"));
        error = true;
        return;
    }

    float a, g;
    SonnenStand(Breite, Laenge, t.Day(), t.Month(), t.Hour(), t.Minute(), 0, a, g);

    char buf[20];
    snprintf(buf, sizeof(buf), "%04u-%02u-%02u %02u:%02u:%02u",
             t.Year(), t.Month(), t.Day(), t.Hour(), t.Minute(), t.Second());
    Serial.print(F("AUTO    "));
    Serial.print(buf);

    error = !sendTarget(a, g);
}

// A full line came over USB (it is in reader.buf).
void handleLine(uint32_t now)
{
    if (mode == MODE_AUTO) {
        Serial.println(F("ignored: switch on AUTO"));
        return;
    }

    float a, g;
    if (!parseAngles(reader.buf, a, g)) {
        Serial.println(F("MATLAB  bad line -> ignored"));
        return;                          // panel stays, timeout keeps running
    }

    Serial.print(F("MATLAB"));
    error      = !sendTarget(a, g);      // good line: also clears a timeout error
    lastMatlab = now;                    // timeout starts again
    timedOut   = false;
}

// Reads all characters that are waiting on USB.
void readSerial(uint32_t now)
{
    while (Serial.available() > 0) {
        char c = (char)Serial.read();
        LineStatus st = lineFeed(reader, c);
        if (st == LINE_READY) {
            handleLine(now);
        } else if (st == LINE_TOO_LONG) {
            Serial.println(F("line too long -> ignored"));
        }
    }
}

void setup()
{
    Serial.begin(115200);
    pinMode(PIN_LED, OUTPUT);
    pinMode(PIN_MODE, INPUT_PULLUP);   // open switch reads HIGH
    Rtc.Begin();                       // only sets pin modes, writes nothing
    if (SEND_I2C) {
        Wire.begin();                  // Nano is I2C master, 100 kHz like MATLAB
    }
    lineClear(reader);

    delay(10);                         // let the pull-up charge the wire (setup only)
    lastRaw    = digitalRead(PIN_MODE);
    rawSince   = millis();
    lastMatlab = millis();             // MATLAB timeout starts at power-on
    mode       = lastRaw ? MODE_AUTO : MODE_MATLAB;   // at start: take it directly

    Serial.print(F("Hybrid firmware, SEND_I2C = "));
    Serial.println(SEND_I2C ? F("ON") : F("OFF"));
    printMode();
}

void loop()
{
    uint32_t now = millis();

    // 1) Switch: did the mode change?
    if (updateMode(now)) {
        error = false;                 // old error belonged to the other mode
        flushInput();                  // old USB data belonged to the other mode
        printMode();
        if (mode == MODE_AUTO) {
            runNow = true;             // back in AUTO: compute right away
        } else {
            lastMatlab = now;          // MATLAB: timeout starts now
            timedOut   = false;        // panel stays until a good line comes
        }
    }

    // 2) USB: handle every full line
    readSerial(now);

    // 3) AUTO: cycle right now, or 30 s after the last one
    if (mode == MODE_AUTO && (runNow || isDue(now, lastCycle, INTERVAL_MS))) {
        runNow    = false;
        lastCycle = now;
        autoCycle();
    }

    // 4) MATLAB: no good line for too long -> report once, fast blink
    if (mode == MODE_MATLAB && !timedOut && isDue(now, lastMatlab, MATLAB_TIMEOUT_MS)) {
        timedOut = true;
        error    = true;
        Serial.println(F("MATLAB  timeout -> panel stays"));
    }

    // 5) LED: on or off right now?
    digitalWrite(PIN_LED, ledOn(mode, error, now) ? HIGH : LOW);
}