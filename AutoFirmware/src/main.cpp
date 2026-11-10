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
#include "Imu.h"
#include "Heading.h"
#include "Correction.h"

static const uint8_t TIME_TEXT_LEN = 20u;

static const float PARK_AZIMUTH_DEG   = 180.0f;
static const float PARK_ELEVATION_DEG = 0.0f;

static const float ACCEL_COUNTS_PER_G = 16384.0f;

static ThreeWire rtcWire(PIN_RTC_DAT, PIN_RTC_CLK, PIN_RTC_RST);
static RtcDS1302<ThreeWire> Rtc(rtcWire);

static Controller controller;
static LineReader reader;
static ClockWatch clockWatch = {false, 0uL, 0uL};

static uint8_t writeRegister(uint8_t registerAddr, uint8_t value);

static bool writeTarget(uint8_t azimuthByte, uint8_t elevationByte);

static bool sendTarget(float azimuthDeg, float elevationDeg);

static bool sendCorrected(float sunAzimuthDeg, float sunElevationDeg, float panelDeg, float correctedDeg);

static bool sendSun(uint32_t now, float azimuthDeg, float elevationDeg);

static void printMode(void);

static void flushInput(void);

static void autoCycle(uint32_t now);

static void handleLine(uint32_t now);

static void readSerial(uint32_t now);

static void printHeading(float headingDeg, bool inError);

static void resendIfTurned(uint32_t now);

static void checkCompass(uint32_t now, uint8_t status);

static void askStatus(uint32_t now);

static uint8_t writeRegister(uint8_t registerAddr, uint8_t value)
{
    Wire.beginTransmission(SLAVE_ADDR);
    Wire.write(registerAddr);
    Wire.write(value);
    return Wire.endTransmission();
}

static bool writeTarget(uint8_t azimuthByte, uint8_t elevationByte)
{
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

static bool sendTarget(float azimuthDeg, float elevationDeg)
{
    uint8_t azimuthByte   = 0u;
    uint8_t elevationByte = 0u;//value intilizatoin
    bool parked = targetBytes(azimuthDeg, elevationDeg, azimuthByte, elevationByte); //hand over the bytes azm,elev(targetBytes fiils them using the & operator)

    Serial.print(F("  az="));     Serial.print(azimuthDeg, 4);
    Serial.print(F("  el="));     Serial.print(elevationDeg, 4);
    Serial.print(F("  azByte=")); Serial.print(azimuthByte);
    Serial.print(F("  elByte=")); Serial.print(elevationByte);
    if (parked) {
        Serial.print(F("  PARK"));
    }

    return writeTarget(azimuthByte, elevationByte);
}

static bool sendCorrected(float sunAzimuthDeg, float sunElevationDeg, float panelDeg, float correctedDeg)
{
    uint8_t azimuthByte   = toUint8(correctedDeg / 2.0f);
    uint8_t elevationByte = toUint8(sunElevationDeg);

    Serial.print(F("  az="));     Serial.print(sunAzimuthDeg, 4);
    Serial.print(F("  el="));     Serial.print(sunElevationDeg, 4);
    Serial.print(F("  panel="));  Serial.print(panelDeg, 1);
    Serial.print(F("  phi="));    Serial.print(correctedDeg, 1);
    Serial.print(F("  azByte=")); Serial.print(azimuthByte);
    Serial.print(F("  elByte=")); Serial.print(elevationByte);

    return writeTarget(azimuthByte, elevationByte);
}

static bool sendSun(uint32_t now, float azimuthDeg, float elevationDeg)
{
    if (!controller.imuSwitchOn) {
        return sendTarget(azimuthDeg, elevationDeg);
    }

    uint8_t azimuthByte   = 0u;
    uint8_t elevationByte = 0u;
    bool    parked        = targetBytes(azimuthDeg, elevationDeg, azimuthByte, elevationByte);
    ctrlSunAngles(controller, azimuthDeg, elevationDeg, parked);

    bool  ackOk   = false;
    float sentDeg = PARK_AZIMUTH_DEG;
    if (parked) {
        ackOk = sendTarget(azimuthDeg, elevationDeg);
    } else if (!ctrlCompassUsable(controller)) {
        Serial.print(F("  no heading ->"));
        ackOk = sendTarget(PARK_AZIMUTH_DEG, PARK_ELEVATION_DEG);
    } else {
        float panelDeg = panelHeading(controller.headingDeg);
        sentDeg = edgeRule(controller.lastSentDeg, correctedAzimuth(azimuthDeg, panelDeg));
        ackOk   = sendCorrected(azimuthDeg, elevationDeg, panelDeg, sentDeg);
    }

    ctrlSendDone(controller, now, sentDeg, ackOk);
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
        (void)Serial.read();
    }
    lineClear(reader);
}

static void autoCycle(uint32_t now)
{
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

    bool sendOk = sendSun(now, azimuthDeg, elevationDeg);
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

    if (ctrlBlocked(controller)) {
        if (controller.fault == FAULT_NONE) {
            Serial.println(F("MATLAB  ignored: IMU switch not known yet"));
        } else {
            Serial.println(F("MATLAB  ignored: fault -> power-cycle"));
        }
        return;
    }

    Serial.print(F("MATLAB"));
    bool sendOk = sendSun(now, azimuthDeg, elevationDeg);
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

static void printHeading(float headingDeg, bool inError)
{
    Serial.print(F("IMU     h="));
    Serial.print(headingDeg, 1);
    Serial.print(F("  panel="));
    Serial.print(panelHeading(headingDeg), 1);
    if (inError) {
        Serial.print(F("  (error)"));
    }
    Serial.println();
}

static void resendIfTurned(uint32_t now)
{
    if (!ctrlResendAllowed(controller)) {
        return;
    }

    float panelDeg     = panelHeading(controller.headingDeg);
    float correctedDeg = edgeRule(controller.lastSentDeg,
                                  correctedAzimuth(controller.sunAzimuthDeg, panelDeg));
    if (!needsResend(controller.lastSentDeg, correctedDeg)) {
        return;
    }

    Serial.print(F("RESEND"));
    bool ackOk = sendCorrected(controller.sunAzimuthDeg, controller.sunElevationDeg, panelDeg, correctedDeg);
    ctrlSendDone(controller, now, correctedDeg, ackOk);
}

static void checkCompass(uint32_t now, uint8_t status)
{
    if (!controller.imuSwitchOn) {
        return;
    }
    if (!ctrlReadCompass(controller, status)) {
        Serial.println(F("IMU     motor moving -> skip"));
        return;
    }

    ImuRaw raw = {0, 0, 0, 0, 0};
    float  headingDeg = -1.0f;
    bool   readOk = imuRead(raw);
    if (readOk) {
        float magX = 0.0f;
        float magY = 0.0f;
        float magZ = 0.0f;
        magCalibrated(raw.magX, raw.magY, raw.magZ, magX, magY, magZ);
        headingDeg = tiltHeading((float)raw.accelX / ACCEL_COUNTS_PER_G,
                                 (float)raw.accelY / ACCEL_COUNTS_PER_G, magX, magY, magZ);
    }

    CompassResult result = ctrlCompassRead(controller, readOk, headingDeg);
    if (result == COMPASS_OK) {
        printHeading(headingDeg, false);
        resendIfTurned(now);
    } else if (result == COMPASS_BAD_IGNORED) {
        Serial.println(F("IMU     bad read (ignored)"));
    } else if (result == COMPASS_FAILED) {
        Serial.print(F("compass failed -> park"));
        bool ackOk = sendTarget(PARK_AZIMUTH_DEG, PARK_ELEVATION_DEG);
        ctrlSendDone(controller, now, PARK_AZIMUTH_DEG, ackOk);
    } else if (result == COMPASS_STILL_FAILED) {
        if (readOk) {
            printHeading(headingDeg, true);
        } else {
            Serial.println(F("IMU     bad read (error)"));
        }
    } else {
        printHeading(headingDeg, false);
        Serial.println(F("compass OK again"));
        resendIfTurned(now);
    }
}

static void askStatus(uint32_t now)
{
    if (!ctrlStatusDue(controller, now)) {
        return;
    }

    uint8_t status   = 0u;
    bool    answered = Wire.requestFrom(SLAVE_ADDR, (uint8_t)1u) == 1u;
    if (answered) {
        status = (uint8_t)Wire.read();
    }

    StatusResult result = ctrlStatus(controller, answered, status, now);
    if (result == RESULT_LATCHED) {
        Serial.println(controller.imuSwitchOn ? F("IMU switch AN") : F("IMU switch AUS"));
    } else if (result == RESULT_FAULT) {
        if (controller.fault == FAULT_NO_ANSWER) {
            Serial.println(F("right Nano not answering -> power-cycle"));
        } else {
            Serial.println(F("IMU switch moved -> power-cycle"));
            Serial.print(F("FAULT "));
            (void)sendTarget(PARK_AZIMUTH_DEG, PARK_ELEVATION_DEG);
        }
    }

    if ((result == RESULT_LATCHED) || (result == RESULT_SAME)) {
        checkCompass(now, status);
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

    if (!SEND_I2C) {
        (void)ctrlStatus(controller, true, 0u, millis());
        Serial.println(F("IMU switch AUS (no I2C)"));
    }
}

void loop()
{
    uint32_t now = millis();

    if (ctrlSwitch(controller, digitalRead(PIN_MODE) == HIGH, now)) {
        flushInput();
        printMode();
    }

    if (SEND_I2C) {
        askStatus(now);
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