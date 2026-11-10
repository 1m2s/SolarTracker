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
#include "ImuAverage.h"
#include "Heading.h"
#include "Correction.h"

static const uint8_t TIME_TEXT_LEN = 20u;

static const float PARK_AZIMUTH_DEG   = 180.0f;
static const float PARK_ELEVATION_DEG = 0.0f;

static const float SOUTH_DEG = 180.0f;

static const uint32_t MS_PER_S = 1000uL;

static const float ACCEL_COUNTS_PER_G = 16384.0f;

static ThreeWire rtcWire(PIN_RTC_DAT, PIN_RTC_CLK, PIN_RTC_RST);
static RtcDS1302<ThreeWire> Rtc(rtcWire);

static Controller controller;
static LineReader reader;
static ClockWatch clockWatch = {false, 0uL, 0uL};
static ImuAverage average;

static uint8_t writeRegister(uint8_t registerAddr, uint8_t value);

static bool writeTarget(uint8_t azimuthByte, uint8_t elevationByte);

static bool sendTarget(float azimuthDeg, float elevationDeg);

static void parkForFault(void);

static bool sendSun(float azimuthDeg, float elevationDeg);

static void printMode(void);

static void flushInput(void);

static void autoCycle(uint32_t now);

static void handleLine(uint32_t now);

static void readSerial(uint32_t now);

static float headingFromRaw(const ImuRaw &raw);

static void aimSouth(uint32_t now);

static void startupTick(uint32_t now);

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

static void parkForFault(void)
{
    Serial.print(F("FAULT "));
    (void)sendTarget(PARK_AZIMUTH_DEG, PARK_ELEVATION_DEG);
}

static bool sendSun(float azimuthDeg, float elevationDeg)
{
    if (!controller.imuSwitchOn) {
        return sendTarget(azimuthDeg, elevationDeg);
    }

    Serial.print(F("  sun="));   Serial.print(azimuthDeg, 1);
    Serial.print(F("  panel=")); Serial.print(controller.panelDeg, 1);
    return sendTarget(correctedAzimuth(azimuthDeg, controller.panelDeg), elevationDeg);
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

    bool sendOk = sendSun(azimuthDeg, elevationDeg);
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
            Serial.println(F("MATLAB  ignored: startup not finished"));
        } else {
            Serial.println(F("MATLAB  ignored: fault -> power-cycle"));
        }
        return;
    }

    Serial.print(F("MATLAB"));
    bool sendOk = sendSun(azimuthDeg, elevationDeg);
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

static float headingFromRaw(const ImuRaw &raw)
{
    float magX = 0.0f;
    float magY = 0.0f;
    float magZ = 0.0f;
    magCalibrated(raw.magX, raw.magY, raw.magZ, magX, magY, magZ);
    return tiltHeading((float)raw.accelX / ACCEL_COUNTS_PER_G,
                       (float)raw.accelY / ACCEL_COUNTS_PER_G, magX, magY, magZ);
}

static void aimSouth(uint32_t now)
{
    ImuRaw  raw           = {0, 0, 0, 0, 0};
    float   sensorDeg     = 0.0f;
    float   panelDeg      = 0.0f;
    float   southDeg      = 0.0f;
    uint8_t azimuthByte   = 0u;
    uint8_t elevationByte = 0u;
    bool    parked        = true;

    bool averageOk = averageResult(average, raw);
    if (averageOk) {
        sensorDeg = headingFromRaw(raw);
        panelDeg  = panelHeading(sensorDeg);
        southDeg  = correctedAzimuth(SOUTH_DEG, panelDeg);
        parked    = alignBytes(southDeg, azimuthByte, elevationByte);
    }

    ImuFault fault = ctrlStartupMeasured(controller, averageOk, !parked, panelDeg, now);
    if (fault == FAULT_COMPASS) {
        Serial.println(F("START   no good compass reads -> power-cycle"));
    } else if (fault == FAULT_NOT_SOUTH) {
        Serial.print(F("START   panel faces north (panel="));
        Serial.print(panelDeg, 1);
        Serial.println(F("), cables can't reach south -> turn the tracker around, power-cycle"));
    }
    if (fault != FAULT_NONE) {
        parkForFault();
        return;
    }

    Serial.print(F("START   reads=")); Serial.print(average.goodReads);
    Serial.print(F("  h="));           Serial.print(sensorDeg, 1);
    Serial.print(F("  panel="));       Serial.print(panelDeg, 1);
    Serial.print(F("  south="));       Serial.print(southDeg, 1);
    Serial.print(F("  azByte="));      Serial.print(azimuthByte);
    Serial.print(F("  elByte="));      Serial.print(elevationByte);
    (void)writeTarget(azimuthByte, elevationByte);
}

static void startupTick(uint32_t now)
{
    StartupStep step = ctrlStartupStep(controller, now);

    switch (step) {
    case STEP_SETTLE:
        Serial.print(F("START   hands off, measuring in "));
        Serial.print(ALIGN_SETTLE_MS / MS_PER_S);
        Serial.println(F(" s"));
        break;

    case STEP_START_MEASURE:
        averageReset(average);
        Serial.print(F("START   measuring "));
        Serial.print(ALIGN_MEASURE_MS / MS_PER_S);
        Serial.println(F(" s"));
        break;

    case STEP_READ: {
        ImuRaw raw    = {0, 0, 0, 0, 0};
        bool   readOk = imuRead(raw);
        averageAdd(average, readOk, raw);
        break;
    }

    case STEP_AIM:
        aimSouth(now);
        break;

    case STEP_FAULT:
        Serial.println(F("START   turn not finished -> power-cycle"));
        parkForFault();
        break;

    case STEP_DONE:
        Serial.println(F("START   done -> running"));
        break;

    case STEP_NONE:
    default:
        break;
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
            parkForFault();
        }
    }

    if ((result == RESULT_LATCHED) || (result == RESULT_SAME)) {
        if (ctrlStartupMotor(controller, status, now)) {
            Serial.print(F("START   motor stopped -> hold "));
            Serial.print(ALIGN_HOLD_MS / MS_PER_S);
            Serial.println(F(" s"));
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
    averageReset(average);

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

    startupTick(now);

    readSerial(now);

    if (ctrlAutoDue(controller, now)) {
        autoCycle(now);
    }

    if (ctrlMatlabTimedOut(controller, now)) {
        Serial.println(F("MATLAB  timeout -> panel stays"));
    }

    digitalWrite(PIN_LED, ctrlLedOn(controller, now) ? HIGH : LOW);
}