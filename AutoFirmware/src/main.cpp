/**
 * @file main.cpp
 * @brief AutoFirmware for the left Nano: sun position (AUTO) or MATLAB angles
 *
 * The switch on PIN_MODE picks the mode. In AUTO the Nano reads the RTC every
 * AUTO_INTERVAL_MS and computes the sun position (AusgabeSonnenstand.m in C++).
 * In MATLAB mode it takes "azimuth elevation" lines from USB. Both modes send
 * the two target bytes to the right Nano over I2C. loop() never blocks.
 *
 * This file only talks to the hardware (switch, LED, USB, I2C, RTC). Every
 * decision (debounce, when to run, timeout, error, LED pattern) is made by
 * Controller, which is tested on the PC.
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
#include "TimeChecks.h"
#include "Controller.h"
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

/* Mode, timers and error state. Set up by ctrlInit() in setup(). */
static Controller controller;
/* Collects USB characters into one line. */
static LineReader reader;
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
 * @param[in] registerAddr  Register, REG_AZIMUTH or REG_ELEVATION.
 * @param[in] value         Byte to write, 0..255.
 *
 * @return Code of Wire.endTransmission(): 0 = OK (ACK), 1 = data too long,
 *         2 = no answer (address NACK), 3 = data NACK, 4 = other error,
 *         5 = timeout (stuck bus, TWI was reset).
 *
 * @note Blocks at most WIRE_TIMEOUT_US.
 */
static uint8_t writeRegister(uint8_t registerAddr, uint8_t value);

/**
 * @brief Send a target to the right Nano and print it
 *
 * Turns the angles into the two bytes (or the park bytes at night), prints
 * "  az=..  el=..  azByte=..  elByte=..  [PARK]  ack=../.." and ends the line.
 *
 * @param[in] azimuthDeg    Azimuth, deg.
 * @param[in] elevationDeg  Elevation, deg.
 *
 * @return true if the right Nano answered OK to both bytes, or SEND_I2C is
 *         false. false if one of the two transfers failed.
 */
static bool sendTarget(float azimuthDeg, float elevationDeg);

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
 * @brief Run one AUTO cycle: read the RTC, compute the sun position, send it
 *
 * Reports a failure to the Controller if the RTC is stopped, not valid or
 * frozen (nothing is sent then), or if the right Nano does not answer.
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
static uint8_t writeRegister(uint8_t registerAddr, uint8_t value)
{
    Wire.beginTransmission(SLAVE_ADDR);
    Wire.write(registerAddr);
    Wire.write(value);
    return Wire.endTransmission();
}

static bool sendTarget(float azimuthDeg, float elevationDeg)
{
    uint8_t azimuthByte   = 0u;
    uint8_t elevationByte = 0u;
    bool parked = targetBytes(azimuthDeg, elevationDeg, azimuthByte, elevationByte);

    Serial.print(F("  az="));     Serial.print(azimuthDeg, 4);
    Serial.print(F("  el="));     Serial.print(elevationDeg, 4);
    Serial.print(F("  azByte=")); Serial.print(azimuthByte);
    Serial.print(F("  elByte=")); Serial.print(elevationByte);
    if (parked) {
        Serial.print(F("  PARK"));
    }

    bool ackOk = true;
    if (SEND_I2C) {
        uint8_t ackAzimuth   = writeRegister(REG_AZIMUTH, azimuthByte);
        uint8_t ackElevation = writeRegister(REG_ELEVATION, elevationByte);
        Serial.print(F("  ack="));
        Serial.print(ackAzimuth);
        Serial.print('/');
        Serial.print(ackElevation);
        ackOk = (ackAzimuth == 0u) && (ackElevation == 0u);
    }
    Serial.println();
    return ackOk;
}

static void printMode(void)
{
    if (controller.mode == MODE_AUTO) {
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

static void autoCycle(uint32_t now)
{
    /*
     * GetDateTime() masks the clock-halt bit, so a stopped clock still gives
     * a valid-looking time. Only GetIsRunning() shows it.
     */
    if (!Rtc.GetIsRunning()) {
        Serial.println(F("AUTO    RTC stopped -> nothing sent"));
        ctrlAutoResult(controller, false);
        return;
    }

    RtcDateTime rtcTime = Rtc.GetDateTime();

    if (!rtcTime.IsValid() || rtcTime.Year() < RTC_YEAR_MIN) {
        Serial.println(F("AUTO    RTC not valid -> nothing sent"));
        ctrlAutoResult(controller, false);
        return;
    }

    /* A valid time that does not move means a dead crystal. */
    if (clockFrozen(clockWatch, now, rtcTime.TotalSeconds())) {
        Serial.println(F("AUTO    RTC frozen -> nothing sent"));
        ctrlAutoResult(controller, false);
        return;
    }

    float azimuthDeg   = 0.0f;
    float elevationDeg = 0.0f;
    sunPosition(LATITUDE_DEG, LONGITUDE_DEG, rtcTime.Day(), rtcTime.Month(),
                rtcTime.Hour(), rtcTime.Minute(), RTC_SUMMER_TIME, azimuthDeg, elevationDeg);

    char timeText[TIME_TEXT_LEN];
    int printedLength = snprintf(timeText, sizeof(timeText), "%04u-%02u-%02u %02u:%02u:%02u",
                                 rtcTime.Year(), rtcTime.Month(), rtcTime.Day(),
                                 rtcTime.Hour(), rtcTime.Minute(), rtcTime.Second());
    Serial.print(F("AUTO    "));
    if ((printedLength > 0) && ((size_t)printedLength < sizeof(timeText))) {
        Serial.print(timeText);
    } else {
        Serial.print(F("(time?)"));
    }

    bool sendOk = sendTarget(azimuthDeg, elevationDeg);
    ctrlAutoResult(controller, sendOk);
}

static void handleLine(uint32_t now)
{
    if (controller.mode == MODE_AUTO) {
        Serial.println(F("ignored: switch on AUTO"));
        return;
    }

    float azimuthDeg   = 0.0f;
    float elevationDeg = 0.0f;
    if (!parseAngles(reader.buf, azimuthDeg, elevationDeg)) {
        Serial.println(F("MATLAB  bad line -> ignored"));
        return;
    }

    Serial.print(F("MATLAB"));
    bool sendOk = sendTarget(azimuthDeg, elevationDeg);
    ctrlMatlabLine(controller, now, sendOk);
}

static void readSerial(uint32_t now)
{
    while (Serial.available() > 0) {
        char received = (char)Serial.read();
        LineStatus lineStatus = lineFeed(reader, received);
        if (lineStatus == LINE_READY) {
            handleLine(now);
        } else if (lineStatus == LINE_TOO_LONG) {
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
    ctrlInit(controller, digitalRead(PIN_MODE) == HIGH, millis());

    Serial.print(F("Hybrid firmware, SEND_I2C = "));
    Serial.println(SEND_I2C ? F("ON") : F("OFF"));
    printMode();
}

void loop()
{
    uint32_t now = millis();

    if (ctrlSwitch(controller, digitalRead(PIN_MODE) == HIGH, now)) {
        /* Old USB data belongs to the other mode. */
        flushInput();
        printMode();
    }

    readSerial(now);

    if (ctrlAutoDue(controller, now)) {
        autoCycle(now);
    }

    if (ctrlMatlabTimedOut(controller, now)) {
        Serial.println(F("MATLAB  timeout -> panel stays"));
    }

    digitalWrite(PIN_LED, ctrlLedOn(controller, now) ? HIGH : LOW);
}