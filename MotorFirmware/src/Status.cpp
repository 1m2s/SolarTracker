#include "Status.h"

uint8_t statusByte(bool imuSwitchOn, bool stepperMoving, bool readingsReady)
{
    uint8_t status = 0u;
    if (imuSwitchOn) {
        status = status | STATUS_IMU_ON;
    }
    if (stepperMoving) {
        status = status | STATUS_MOVING;
    }
    if (readingsReady) {
        status = status | STATUS_READINGS;
    }
    return status;
}