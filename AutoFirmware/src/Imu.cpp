#include "Imu.h"
#include "config/Board.h"
#include "config/Settings.h"
#include <Arduino.h>
#include <Wire.h>

static const uint8_t REG_ACCEL_CONFIG = 0x1Cu;
static const uint8_t REG_INT_PIN_CFG  = 0x37u;
static const uint8_t REG_ACCEL_XOUT_H = 0x3Bu;
static const uint8_t REG_PWR_MGMT_1   = 0x6Bu;
static const uint8_t REG_WHO_AM_I     = 0x75u;

static const uint8_t REG_MAG_WIA   = 0x00u;
static const uint8_t REG_MAG_HXL   = 0x03u;
static const uint8_t REG_MAG_CNTL1 = 0x0Au;

static const uint8_t IMU_ID          = 0x71u;
static const uint8_t MAG_ID          = 0x48u;
static const uint8_t CLOCK_FROM_GYRO = 0x01u;
static const uint8_t ACCEL_RANGE_2G  = 0x00u;
static const uint8_t BYPASS_ON       = 0x02u;
static const uint8_t MAG_MEASURE_ONE = 0x11u;
static const uint8_t MAG_OVERFLOW    = 0x08u;

static const uint8_t ACCEL_BYTES     = 4u;
static const uint8_t MAG_BYTES       = 7u;
static const uint8_t MAG_STATUS_BYTE = 6u;

static const uint32_t WAKE_UP_MS = 10uL;

static bool needsWakeUp = true;

static bool writeRegister(uint8_t device, uint8_t reg, uint8_t value)
{
    Wire.beginTransmission(device);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0u;
}

static bool readRegisters(uint8_t device, uint8_t reg, uint8_t count, uint8_t *buffer)
{
    Wire.beginTransmission(device);
    Wire.write(reg);
    if (Wire.endTransmission() != 0u) {
        return false;
    }
    if (Wire.requestFrom(device, count) != count) {
        return false;
    }
    for (uint8_t index = 0u; index < count; index++) {
        buffer[index] = (uint8_t)Wire.read();
    }
    return true;
}

static int16_t joinBytes(uint8_t highByte, uint8_t lowByte)
{
    uint16_t joined = (uint16_t)(((uint16_t)highByte << 8u) | lowByte);
    return (int16_t)joined;
}

static bool readMagOnce(int16_t &magX, int16_t &magY, int16_t &magZ)
{
    uint8_t buffer[MAG_BYTES] = {0u, 0u, 0u, 0u, 0u, 0u, 0u};

    if (!writeRegister(MAG_ADDR, REG_MAG_CNTL1, MAG_MEASURE_ONE)) {
        return false;
    }
    delay(MAG_WAIT_MS);
    if (!readRegisters(MAG_ADDR, REG_MAG_HXL, MAG_BYTES, buffer)) {
        return false;
    }
    if ((buffer[MAG_STATUS_BYTE] & MAG_OVERFLOW) != 0u) {
        return false;
    }

    magX = joinBytes(buffer[1], buffer[0]);
    magY = joinBytes(buffer[3], buffer[2]);
    magZ = joinBytes(buffer[5], buffer[4]);

    if ((magX == 0) && (magY == 0) && (magZ == 0)) {
        return false;
    }
    return true;
}

bool imuWhoAmI(uint8_t &id)
{
    return readRegisters(IMU_ADDR, REG_WHO_AM_I, 1u, &id);
}

bool imuBegin()
{
    uint8_t imuId = 0u;
    uint8_t magId = 0u;

    if (!imuWhoAmI(imuId) || (imuId != IMU_ID)) {
        return false;
    }
    if (!writeRegister(IMU_ADDR, REG_PWR_MGMT_1, CLOCK_FROM_GYRO)) {
        return false;
    }
    if (!writeRegister(IMU_ADDR, REG_ACCEL_CONFIG, ACCEL_RANGE_2G)) {
        return false;
    }
    if (!writeRegister(IMU_ADDR, REG_INT_PIN_CFG, BYPASS_ON)) {
        return false;
    }
    delay(WAKE_UP_MS);
    if (!readRegisters(MAG_ADDR, REG_MAG_WIA, 1u, &magId) || (magId != MAG_ID)) {
        return false;
    }

    needsWakeUp = false;
    return true;
}

bool imuRead(ImuRaw &raw)
{
    uint8_t accel[ACCEL_BYTES] = {0u, 0u, 0u, 0u};
    int32_t sumX = 0;
    int32_t sumY = 0;
    int32_t sumZ = 0;

    if (needsWakeUp && !imuBegin()) {
        return false;
    }
    if (!readRegisters(IMU_ADDR, REG_ACCEL_XOUT_H, ACCEL_BYTES, accel)) {
        needsWakeUp = true;
        return false;
    }

    for (uint8_t sample = 0u; sample < MAG_SAMPLES; sample++) {
        int16_t magX = 0;
        int16_t magY = 0;
        int16_t magZ = 0;
        if (!readMagOnce(magX, magY, magZ)) {
            needsWakeUp = true;
            return false;
        }
        sumX += magX;
        sumY += magY;
        sumZ += magZ;
    }

    raw.accelX = joinBytes(accel[0], accel[1]);
    raw.accelY = joinBytes(accel[2], accel[3]);
    raw.magX   = (int16_t)(sumX / (int32_t)MAG_SAMPLES);
    raw.magY   = (int16_t)(sumY / (int32_t)MAG_SAMPLES);
    raw.magZ   = (int16_t)(sumZ / (int32_t)MAG_SAMPLES);
    return true;
}