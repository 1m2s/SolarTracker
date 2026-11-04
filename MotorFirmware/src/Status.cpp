#include "Status.h"

uint8_t statusByte(bool imuSwitchOn, bool stepperMoving)
{
    uint8_t status = 0u;
    if (imuSwitchOn) {
        status = status | STATUS_IMU_ON;
    }
    if (stepperMoving) {
        status = status | STATUS_MOVING;
    }
    return status;
}