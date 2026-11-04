#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdint.h>

const bool SEND_I2C = true;

const uint32_t SERIAL_BAUD = 115200uL;

const uint32_t AUTO_INTERVAL_MS = 30000uL;

const uint32_t DEBOUNCE_MS = 50uL;

const uint32_t MATLAB_TIMEOUT_MS = 180000uL;

const uint32_t PULLUP_SETTLE_MS = 10uL;

const float LATITUDE_DEG  = 47.81f;
const float LONGITUDE_DEG = 9.66f;

const uint16_t RTC_YEAR_MIN = 2026u;

const bool RTC_SUMMER_TIME = false;

const uint8_t MAG_SAMPLES = 4u;

const uint32_t MAG_WAIT_MS = 10uL;

const float RESEND_DEG      = 4.0f;
const float EDGE_MARGIN_DEG = 10.0f;

#endif