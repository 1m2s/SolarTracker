#ifndef BOARD_H
#define BOARD_H

#include <stdint.h> //include for uint8_t(8bits 0..255 ), uint32_t()
//while variable declaring u is used to indicate unsigned int , and L is used to indicate long int (32bit)

const uint8_t PIN_MODE = 2u;  //Mode switch (AUTO/MATLAB) ON D2 (main.cpp 351,360,376)

const uint8_t PIN_LED = 13u; //Status LED on D13 (main.cpp 350 395)

//DS1302 (main.cpp 23)
const uint8_t PIN_RTC_CLK = 5u; //RTC clock line on D5
const uint8_t PIN_RTC_DAT = 6u; //RTC data line on D6
const uint8_t PIN_RTC_RST = 7u; //RTC reset/start talking line on D7

const uint8_t SLAVE_ADDR = 0x19u; //The Right nano's I2C address (main.cpp 60,324)

const uint8_t REG_AZIMUTH   = 254u; //The register address for azimuth angle(Mailbox for azimuth valuex2=degrees) (main.cpp70)
const uint8_t REG_ELEVATION = 255u; //The register address for elevation angle (main.cpp 71)

//Used by bitmasks so if status returns as 0x03 then both bits are set status&(either)=imu switch is on and stepper is moving
const uint8_t STATUS_IMU_ON = 0x01u; //Bit 0 of the status byte indicating switch is on (Controller.cpp 133)
const uint8_t STATUS_MOVING = 0x02u; //Bit 1 = stepper is moving (Controller.cpp 220)

/*so the left nano has 3 i2c addresses: 0x19(right nano), 0x68(imu) , 0x0C(compass)
Both used in Imu.cpp
*/
const uint8_t IMU_ADDR = 0x68u; //MPU-9250(accelerometer +. gyro)
const uint8_t MAG_ADDR = 0x0Cu; //AK8963 magnetometer (compass)


/* i2c safety timeout in microseconds (25ms) and the reason is a device on the bus hangs
a plain i2c call would wait forever and the whole tracker will freeze 
used in main.cpp line 355
*/
const uint32_t WIRE_TIMEOUT_US = 25000uL;

#endif

/*
this file contains facts about hardware
5 pins on the left nano are used for the RTC (DS1302) and the mode switch and the status LED
the right nanos i2c address and its 2 mailboxes and 2 status bits 
the imu's 2 addresses 
the i2c timeout const
*/