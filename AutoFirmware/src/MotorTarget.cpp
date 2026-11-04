#include <math.h>
#include "MotorTarget.h"

static const uint8_t PARK_AZIMUTH_BYTE   = 90u; //right nano reads it as 90*2=180deg=south
static const uint8_t PARK_ELEVATION_BYTE = 0u; //vertical position

static const float AZIMUTH_SAFE_MIN_DEG = 50.0f;
static const float AZIMUTH_SAFE_MAX_DEG = 310.0f;
//safe azimuth range is 50deg to 310 

uint8_t toUint8(float value) //turn any float into a byte safely(0..255)
{
    if (!(value > 0.0f)) {
        return 0u;
    } //its true for negative mumbers and NaN
    if (value >= 255.0f) {
        return 255u;
    }//cap at 255
    return (uint8_t)roundf(value);//cast to uint8_t after rounding to nearest whole number
}

bool targetBytes(float azimuthDeg, float elevationDeg, uint8_t &azimuthByte, uint8_t &elevationByte)
{
    bool night  = !(elevationDeg > 0.0f); //true if sun is not above horizon ie 0 or negative elevation and NaN counts as night too 
    bool unsafe = !(azimuthDeg >= AZIMUTH_SAFE_MIN_DEG && azimuthDeg <= AZIMUTH_SAFE_MAX_DEG); //true if azimuth is outside safe range (&&)=>AND operator

    if (night || unsafe) {
        azimuthByte   = PARK_AZIMUTH_BYTE;
        elevationByte = PARK_ELEVATION_BYTE;
        return true; //true if its either night or unsafe so it writes 90,0 into the caller variable and returnrs (ture) indicating yes im parked
    }

    azimuthByte   = toUint8(azimuthDeg / 2.0f);
    elevationByte = toUint8(elevationDeg);
    return false;//we write to both caller variable the deg/2(because azimuth is 0-360 but one byte can only hold 0-255) but elevation is from (0.90) so it fits in a byte as is, and return false indicating its not parked
}
//in words u give it two angles it saftey checks them in, it fills in 2 caller variable bytes and returns if it had to park or not 