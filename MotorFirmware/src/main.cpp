#include <Arduino.h>
#include <Wire.h>
#include <Stepper.h>
#include <Servo.h>
#include <LedControl.h>
#include <util/atomic.h>
#include "Motion.h"
#include "Measure.h"
#include "Display.h"
#include "Status.h"
#include "Readings.h"

const uint8_t SLAVE_ADDR     = 0x19;
const float   SNull          = 95;
const float   SKorr          = 1.2f;
const float   STEP_DEG       = 1.8f;
const uint8_t PIN_IMU_SWITCH = 7;

const uint32_t DISPLAY_WAKE_MS = 1000uL;
const bool     PRINT_SUMS      = true;

Stepper    stepper_1(200, 5, 4, 3, 2);
Servo      servo_2;
LedControl lc = LedControl(10, 12, 11, 1);

volatile uint8_t a8 = 90;
volatile uint8_t g8 = 0;

volatile uint8_t stepperMoving = 0;

static volatile uint16_t readySumA0 = 0u;
static volatile uint16_t readySumA1 = 0u;
static volatile uint8_t  sumsReady  = 0u;

float off1 = 0;
float off2 = 0;

static ReadingSums sums       = {0u, 0u, 0u};
static uint32_t    lastWakeMs = 0uL;

void onMessage(int howMany)
{
    if (howMany != 2) {
        while (Wire.available() > 0) {
            Wire.read();
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

void onAsk()
{
    bool    imuOn  = (digitalRead(PIN_IMU_SWITCH) == LOW);
    bool    ready  = (sumsReady != 0u);
    uint8_t status = statusByte(imuOn, stepperMoving != 0, ready);

    Answer answer;
    packAnswer(status, readySumA0, readySumA1, answer);
    Wire.write(answer.bytes, ANSWER_LENGTH);
}

void setup()
{
    Wire.begin(SLAVE_ADDR);
    Wire.setClock(100000);
    Wire.onReceive(onMessage);
    Wire.onRequest(onAsk);
    pinMode(PIN_IMU_SWITCH, INPUT);
    Serial.begin(115200);

    stepper_1.setSpeed(6);
    servo_2.attach(9);

    wakeDisplay(lc);
    lc.clearDisplay(0);
    lastWakeMs = millis();

    Serial.println(F("MotorFirmware (ohne IMU)"));
}

void loop()
{
    float azi = 2.0f * a8;
    float gho = g8;

    int8_t dir = stepDirection(azi, off1);
    stepperMoving = (dir != 0) ? 1 : 0;
    if (dir != 0) {
        stepper_1.step(dir);
        off1 += dir * STEP_DEG;
    }
    delay(60);

    off2 = servoStep(gho, off2);
    servo_2.write((int)(SNull + off2 * SKorr));

    int raw0 = analogRead(A0);
    int raw1 = analogRead(A1);
    float Up, Pp;
    measure(raw0, raw1, Up, Pp);
    showVoltage(lc, round(Up));
    showPower(lc, round(Pp));

    uint16_t doneA0 = 0u;
    uint16_t doneA1 = 0u;
    if (addReading(sums, raw0, raw1, doneA0, doneA1)) {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            readySumA0 = doneA0;
            readySumA1 = doneA1;
            sumsReady  = 1u;
        }
        if (PRINT_SUMS) {
            Serial.print(F("R:A0="));
            Serial.print(doneA0);
            Serial.print(F(" A1="));
            Serial.println(doneA1);
        }
    }

    uint32_t now = millis();
    if ((uint32_t)(now - lastWakeMs) >= DISPLAY_WAKE_MS) {
        lastWakeMs = now;
        wakeDisplay(lc);
    }
}