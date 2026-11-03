/**
 * @file Board.h
 * @brief Pins and I2C wiring of the left Nano (A_MATLAB)
 *
 * Everything that depends on how the board is wired lives here. When the
 * header PCB moves a pin, only this file changes.
 *
 * Target: Arduino Nano (ATmega328P), Arduino framework (PlatformIO atmelavr). Level: L2.
 *
 * @author Mohamed Sellami
 * @date 2026-11-03
 *
 */
#ifndef BOARD_H
#define BOARD_H

#include <stdint.h>

/******************************************************************************
 Constants
******************************************************************************/
/* Mode switch. Open = HIGH (internal pull-up) = AUTO, closed to GND = MATLAB. */
const uint8_t PIN_MODE = 2u;

/* Status LED, shared with the built-in LED of the Nano. */
const uint8_t PIN_LED = 13u;

/* DS1302 real-time clock, 3-wire interface. */
const uint8_t PIN_RTC_CLK = 5u;
const uint8_t PIN_RTC_DAT = 6u;
const uint8_t PIN_RTC_RST = 7u;

/* I2C address of the right Nano (MotorFirmware). MATLAB uses the same one. */
const uint8_t SLAVE_ADDR = 0x19u;

/* Registers of the right Nano: azimuth byte (a8) and elevation byte (g8). */
const uint8_t REG_ALPHA = 254u;
const uint8_t REG_GAMMA = 255u;

/*
 * A transfer that takes longer than this is given up and the TWI is reset.
 * One 3-byte transfer at 100 kHz takes about 0.3 ms, so 25 ms only trips
 * on a stuck bus.
 */
const uint32_t WIRE_TIMEOUT_US = 25000uL;

#endif /* BOARD_H */