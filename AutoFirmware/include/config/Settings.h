/**
 * @file Settings.h
 * @brief Behaviour settings of AutoFirmware: timings, location, limits
 *
 * Values that decide how the firmware behaves, independent of the wiring.
 * Pins and bus addresses are in Board.h.
 *
 * Target: Arduino Nano (ATmega328P), Arduino framework (PlatformIO atmelavr). Level: L2.
 *
 * @author Mohamed Sellami
 * @date 2026-11-03
 *
 */
#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdint.h>

/******************************************************************************
 Constants
******************************************************************************/
/* false = only print on Serial, nothing is sent and the motors stay still. */
const bool SEND_I2C = true;

/* Serial speed for USB, in baud. AusgabeSonnenstandSerial.m uses the same. */
const uint32_t SERIAL_BAUD = 115200uL;

/* Time between two AUTO cycles. Same as pause(30) in AusgabeSonnenstand.m. */
const uint32_t AUTO_INTERVAL_MS = 30000uL;

/* The switch must stay unchanged this long before a new mode is accepted. */
const uint32_t DEBOUNCE_MS = 50uL;

/* No good MATLAB line for this long = timeout (6 missed lines of 30 s). */
const uint32_t MATLAB_TIMEOUT_MS = 180000uL;

/* Time for the pull-up to charge the switch wire before the first read. */
const uint32_t PULLUP_SETTLE_MS = 10uL;

/* Location of the tracker, same values as in AusgabeSonnenstand.m. */
const float LATITUDE_DEG  = 47.81f;
const float LONGITUDE_DEG = 9.66f;

/* An RTC year below this means the clock was never set or lost its time. */
const uint16_t RTC_YEAR_MIN = 2026u;

/*
 * Does the RTC show summer time? false = CET (winter time), true = CEST.
 * The RTC is always set to CET, so this stays false all year.
 */
const bool RTC_SUMMER_TIME = false;

#endif /* SETTINGS_H */