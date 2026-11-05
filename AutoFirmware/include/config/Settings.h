#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdint.h>
/*when declaring a float f is used to indicate floar otherwise it would be treated as a double
millis() counts in uint32_t so the time uses xUL as the type
*/

//in setup()353 i2c starts only if true and writeTarget()69 only sends bytes if its true 
const bool SEND_I2C = true; //false means a dry run nano computes and prints but never touches I2C so right isnt asked (main.cpp 69,353,362-366,381)

const uint32_t SERIAL_BAUD = 115200uL; //bit/s the PlatformIO and matlab need to use the same number or else they show garbage(main.cpp 349)

const uint32_t AUTO_INTERVAL_MS = 30000uL; //AUTO mode interval between sends 30s (Controller.cpp69)

const uint32_t DEBOUNCE_MS = 50uL; //debounce time 50ms:switch much stay still for 50ms before mode changes (Controller.cpp45)

const uint32_t MATLAB_TIMEOUT_MS = 180000uL; //matlab timeout no good line for 3min (Controller.cpp94)

//as before reading the inputpullup the pin needs a moment to reach high(5v when the switch is open) so the code waits 10ms before reading it 
const uint32_t PULLUP_SETTLE_MS = 10uL; //wait 10ms once in setup() before the first time read of the mode switch (Controller.cpp94)

//used in main.cpp 183
const float LATITUDE_DEG  = 47.81f; //weingarten degrees North
const float LONGITUDE_DEG = 9.66f; //Weingarten degrees east

const uint16_t RTC_YEAR_MIN = 2026u;//clock year as if the clock says a diffrent number then it refuses sun calculation (main.cpp169)

const bool RTC_SUMMER_TIME = false;//clock is kept in winter time all year (main.cpp184)

//together thee delay is 4x10ms=40ms per compass reading , only place the uses a delay() in the loop
const uint8_t MAG_SAMPLES = 4u;//each compass reading= 4 measurments averaged for better noise managment(Imu.cpp134,149-151)
const uint32_t MAG_WAIT_MS = 10uL;//Wait for the compass chip to finish one measurment(Imu.cpp70)

//Controller.cpp 118=>uint32_t intervalMs = controller.imuKnown ? COMPASS_CHECK_MS : STATUS_RETRY_MS; here it says if the imu switch is known wait 2s otherwise 250ms
const uint32_t COMPASS_CHECK_MS = 2000uL;//asks the right nano for the status byte(read the compass if the imu is on) every 2s
const uint32_t STATUS_RETRY_MS  = 250uL; //in the beginning when imu switch position is unkown then ask every 250ms

const uint32_t STARTUP_WAIT_MS  = 5000uL;//if the right nano hasnt answered within 5s of start then flag a error(Controller.cpp141)

const uint8_t COMPASS_FAIL_READS    = 3u; //3bad compass readings in a row then compassfailed and park (Controller.cpp185,191)
const uint8_t COMPASS_RECOVER_READS = 3u; //3good readings in a row then it recovers and tracking continues(Controller.cpp171,177)

const float RESEND_DEG      = 6.0f; //if someone turns the base and the corrected angle changes by 6deg or more then send again (small changes ignored due to noise)(Correction.cpp50)
const float EDGE_MARGIN_DEG = 10.0f; //Edge rule around 0/360(North) only switch once the new angle passes 10deg this stops the mast from fliping a full turn back and forth around North(Correction.cpp33,36) 

#endif

/*
This file holds numbers u can tune:
test switch
baudrate
mode timings
location and clock rules
compass reading
status timing
compass error tolerance
imu correction limits
*/