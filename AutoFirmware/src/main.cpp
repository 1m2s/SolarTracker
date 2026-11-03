/**
 * @file main.cpp
 * @brief AutoFirmware for the left Nano: sun position (AUTO) or MATLAB angles
 *
 * The switch on PIN_MODE picks the mode. In AUTO the Nano reads the RTC every
 * AUTO_INTERVAL_MS and computes the sun position (AusgabeSonnenstand.m in C++).
 * In MATLAB mode it takes "alpha gamma" lines from USB. Both modes send the
 * two target bytes to the right Nano over I2C. loop() never blocks.
 *
 * Target: Arduino Nano (ATmega328P), Arduino framework (PlatformIO atmelavr). Level: L2.
 *
 * @author Mohamed Sellami
 * @date 2026-11-03
 *
 */
#include <Arduino.h>
#include <Wire.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>
#include "config/Board.h"
#include "config/Settings.h"
#include "SunPosition.h"
#include "MotorTarget.h"
#include "LedPattern.h"
#include "TimeChecks.h"
#include "MatlabLink.h"

/******************************************************************************
 Constants
******************************************************************************/
/* "YYYY-MM-DD hh:mm:ss" is 19 characters plus the end marker. */
static const uint8_t TIME_TEXT_LEN = 20u;

/******************************************************************************
 Private Data
******************************************************************************/
/* RTC driver. The constructors only store the pins; Rtc.Begin() in setup() sets them up. */
static ThreeWire rtcWire(PIN_RTC_DAT, PIN_RTC_CLK, PIN_RTC_RST);
static RtcDS1302<ThreeWire> Rtc(rtcWire);

/*
 * State kept between loop() passes. Written and read only by loop() and the
 * functions it calls; no ISR uses it, so no protection is needed.
 */

/* Mode the firmware acts on (after debounce). */
static Mode mode = MODE_AUTO;
/* true = the last cycle failed, the LED blinks fast. */
static bool error = false;
/* true = run an AUTO cycle now instead of waiting AUTO_INTERVAL_MS. */
static bool runNow = true;
/* millis() of the last AUTO cycle. */
static uint32_t lastCycle = 0uL;
/* Last raw reading of PIN_MODE. */
static bool lastRaw = HIGH;
/* millis() when that raw reading last changed. */
static uint32_t rawSince = 0uL;
/* Collects USB characters into one line. */
static LineReader reader;
/* millis() of the last good MATLAB line. */
static uint32_t lastMatlab = 0uL;
/* true = the MATLAB timeout was already reported. */
static bool timedOut = false;
/* Last RTC reading, to see if the clock still moves. */
static ClockWatch clockWatch = {false, 0uL, 0uL};

/******************************************************************************
 Private Function Declarations
******************************************************************************/
/**
 * @brief Write one byte into a register of the right Nano
 *
 * Same as writeRegister(slave, reg, value, 'uint8') in MATLAB.
 *
 * @param[in] reg    Register, REG_ALPHA or REG_GAMMA.
 * @param[in] value  Byte to write, 0..255.
 *
 * @return Code of Wire.endTransmission(): 0 = OK (ACK), 1 = data too long,
 *         2 = no answer (address NACK), 3 = data NACK, 4 = other error,
 *         5 = timeout (stuck bus, TWI was reset).
 *
 * @note Blocks at most WIRE_TIMEOUT_US.
 */
static uint8_t writeRegister(uint8_t reg, uint8_t value);

/**
 * @brief Send a target to the right Nano and print it
 *
 * Turns the angles into the two bytes (or the park bytes at night), prints
 * "  a=..  g=..  a8=..  g8=..  [PARK]  ack=../.." and ends the line.
 *
 * @param[in] a  Azimuth alpha, deg.
 * @param[in] g  Elevation gamma, deg.
 *
 * @return true if the right Nano answered OK to both bytes, or SEND_I2C is
 *         false. false if one of the two transfers failed.
 */
static bool sendTarget(float a, float g);

/**
 * @brief Print the current mode on Serial
 */
static void printMode(void);

/**
 * @brief Throw away all USB input that was not used yet
 *
 * Empties the receive buffer of the Nano and the half line in the LineReader.
 */
static void flushInput(void);

/**
 * @brief Read the switch and accept a new mode after debounce
 *
 * A new switch position counts only after it stayed unchanged for DEBOUNCE_MS.
 *
 * @param[in] now  millis() of this loop pass.
 *
 * @return true if the mode changed in this call, false otherwise.
 */
static bool updateMode(uint32_t now);

/**
 * @brief Run one AUTO cycle: read the RTC, compute the sun position, send it
 *
 * Sets error to true if the RTC is stopped, not valid or frozen (nothing is
 * sent then), or if the right Nano does not answer.
 *
 * @param[in] now  millis() of this loop pass.
 */
static void autoCycle(uint32_t now);

/**
 * @brief Handle one full line from USB (in reader.buf)
 *
 * In AUTO the line is ignored. In MATLAB mode a good line is sent and
 * restarts the timeout; a broken line is ignored and the timeout keeps running.
 *
 * @param[in] now  millis() of this loop pass.
 */
static void handleLine(uint32_t now);

/**
 * @brief Read every character waiting on USB and handle full lines
 *
 * @param[in] now  millis() of this loop pass.
 */
static void readSerial(uint32_t now);

/******************************************************************************
 Function Definitions
******************************************************************************/
static uint8_t writeRegister(uint8_t reg, uint8_t value)
{
    Wire.beginTransmission(SLAVE_ADDR);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission();
}

static bool sendTarget(float a, float g)
{
    uint8_t a8 = 0u;
    uint8_t g8 = 0u;
    bool parked = targetBytes(a, g, a8, g8);

    Serial.print(F("  a="));  Serial.print(a, 4);
    Serial.print(F("  g="));  Serial.print(g, 4);
    Serial.print(F("  a8=")); Serial.print(a8);
    Serial.print(F("  g8=")); Serial.print(g8);
    if (parked) {
        Serial.print(F("  PARK"));
    }

    bool ackOk = true;
    if (SEND_I2C) {
        uint8_t ackA = writeRegister(REG_ALPHA, a8);
        uint8_t ackG = writeRegister(REG_GAMMA, g8);
        Serial.print(F("  ack="));
        Serial.print(ackA);
        Serial.print('/');
        Serial.print(ackG);
        ackOk = (ackA == 0 && ackG == 0);
    }
    Serial.println();
    return ackOk;
}

static void printMode(void)
{
    if (mode == MODE_AUTO) {
        Serial.println(F("Mode -> AUTO"));
    } else {
        Serial.println(F("Mode -> MATLAB (waiting for a line)"));
    }
}

static void flushInput(void)
{
    while (Serial.available() > 0) {
        /* The byte is not needed; reading it is what removes it. */
        (void)Serial.read();
    }
    lineClear(reader);
}

static bool updateMode(uint32_t now)
{
    bool raw = digitalRead(PIN_MODE);
    if (raw != lastRaw) {
        lastRaw  = raw;
        rawSince = now;
        return false;
    }
    Mode m = raw ? MODE_AUTO : MODE_MATLAB;
    if (m != mode && isDue(now, rawSince, DEBOUNCE_MS)) {
        mode = m;
        return true;
    }
    return false;
}

static void autoCycle(uint32_t now)
{
    /*
     * GetDateTime() masks the clock-halt bit, so a stopped clock still gives
     * a valid-looking time. Only GetIsRunning() shows it.
     */
    if (!Rtc.GetIsRunning()) {
        Serial.println(F("AUTO    RTC stopped -> nothing sent"));
        error = true;
        return;
    }

    RtcDateTime t = Rtc.GetDateTime();

    if (!t.IsValid() || t.Year() < RTC_YEAR_MIN) {
        Serial.println(F("AUTO    RTC not valid -> nothing sent"));
        error = true;
        return;
    }

    /* A valid time that does not move means a dead crystal. */
    if (clockFrozen(clockWatch, now, t.TotalSeconds())) {
        Serial.println(F("AUTO    RTC frozen -> nothing sent"));
        error = true;
        return;
    }

    float a = 0.0f;
    float g = 0.0f;
    SonnenStand(BREITE_DEG, LAENGE_DEG, t.Day(), t.Month(), t.Hour(), t.Minute(), CLOCK_TF, a, g);

    char buf[TIME_TEXT_LEN];
    int n = snprintf(buf, sizeof(buf), "%04u-%02u-%02u %02u:%02u:%02u",
                     t.Year(), t.Month(), t.Day(), t.Hour(), t.Minute(), t.Second());
    Serial.print(F("AUTO    "));
    if ((n > 0) && ((size_t)n < sizeof(buf))) {
        Serial.print(buf);
    } else {
        Serial.print(F("(time?)"));
    }

    error = !sendTarget(a, g);
}

static void handleLine(uint32_t now)
{
    if (mode == MODE_AUTO) {
        Serial.println(F("ignored: switch on AUTO"));
        return;
    }

    float a = 0.0f;
    float g = 0.0f;
    if (!parseAngles(reader.buf, a, g)) {
        Serial.println(F("MATLAB  bad line -> ignored"));
        return;
    }

    Serial.print(F("MATLAB"));
    error      = !sendTarget(a, g);
    lastMatlab = now;
    timedOut   = false;
}

static void readSerial(uint32_t now)
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
    Serial.begin(SERIAL_BAUD);
    pinMode(PIN_LED, OUTPUT);
    pinMode(PIN_MODE, INPUT_PULLUP);
    Rtc.Begin();
    if (SEND_I2C) {
        Wire.begin();
        Wire.setWireTimeout(WIRE_TIMEOUT_US, true);
    }
    lineClear(reader);

    delay(PULLUP_SETTLE_MS);
    lastRaw    = digitalRead(PIN_MODE);
    rawSince   = millis();
    lastMatlab = millis();
    /* At power-on there is no old mode to bounce from, so take it directly. */
    mode       = lastRaw ? MODE_AUTO : MODE_MATLAB;

    Serial.print(F("Hybrid firmware, SEND_I2C = "));
    Serial.println(SEND_I2C ? F("ON") : F("OFF"));
    printMode();
}

void loop()
{
    uint32_t now = millis();

    if (updateMode(now)) {
        /* An old error and old USB data belong to the other mode. */
        error = false;
        flushInput();
        printMode();
        if (mode == MODE_AUTO) {
            runNow = true;
        } else {
            lastMatlab = now;
            timedOut   = false;
        }
    }

    readSerial(now);

    if (mode == MODE_AUTO && (runNow || isDue(now, lastCycle, AUTO_INTERVAL_MS))) {
        runNow    = false;
        lastCycle = now;
        autoCycle(now);
    }

    if (mode == MODE_MATLAB && !timedOut && isDue(now, lastMatlab, MATLAB_TIMEOUT_MS)) {
        timedOut = true;
        error    = true;
        Serial.println(F("MATLAB  timeout -> panel stays"));
    }

    digitalWrite(PIN_LED, ledOn(mode, error, now) ? HIGH : LOW);
}