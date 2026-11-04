#ifndef BOARD_H
#define BOARD_H

#include <stdint.h>

const uint8_t PIN_MODE = 2u;

const uint8_t PIN_LED = 13u;

const uint8_t PIN_RTC_CLK = 5u;
const uint8_t PIN_RTC_DAT = 6u;
const uint8_t PIN_RTC_RST = 7u;

const uint8_t SLAVE_ADDR = 0x19u;

const uint8_t REG_AZIMUTH   = 254u;
const uint8_t REG_ELEVATION = 255u;

const uint8_t IMU_ADDR = 0x68u;
const uint8_t MAG_ADDR = 0x0Cu;

const uint32_t WIRE_TIMEOUT_US = 25000uL;

#endif