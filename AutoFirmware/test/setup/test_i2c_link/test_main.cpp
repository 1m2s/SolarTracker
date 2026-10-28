/* Notes
   Link check: left Nano (master) -> right Nano (slave 0x19).
   It WRITES to the right Nano, so it lives in setup/.
   It only sends the right Nano's start values:
   azimuth 180 deg (a8 = 90) and elevation 0 deg (g8 = 0).
   After power-on the right Nano is already there, so nothing moves.
   Needs: mode "ohne IMU", fresh power-on, panel vertical and facing south.
   Run: pio test -e nanosetup -f setup/test_i2c_link
*/

#include <Arduino.h>
#include <Wire.h>
#include <unity.h>

const uint8_t SLAVE_ADDR = 0x19;   // right Nano (A_AIDE)
const uint8_t EMPTY_ADDR = 0x20;   // nobody here (I2C scan: only 0x19, 0x68, 0x77)

// Answer codes of Wire.endTransmission():
// 0 = OK (ACK), 2 = nobody answered the address, 3 = nobody took the data
uint8_t writeRegister(uint8_t reg, uint8_t value)
{
  Wire.beginTransmission(SLAVE_ADDR);
  Wire.write(reg);
  Wire.write(value);
  return Wire.endTransmission();
}

void setUp(void) {}
void tearDown(void) {}

// Control test: an empty address must NOT answer.
// This proves our check can really fail.
void test_empty_address_gives_nack(void)
{
  Wire.beginTransmission(EMPTY_ADDR);   // only the address, no data
  TEST_ASSERT_EQUAL_UINT8(2, Wire.endTransmission());
}

void test_slave_acks_azimuth(void)
{
  TEST_ASSERT_EQUAL_UINT8(0, writeRegister(254, 90));   // 90 * 2 = 180 deg = south
}

void test_slave_acks_elevation(void)
{
  TEST_ASSERT_EQUAL_UINT8(0, writeRegister(255, 0));    // 0 deg = panel vertical
}

void setup()
{
  delay(2000);         // wait for the Serial monitor
  Wire.begin();        // Nano is I2C master, 100 kHz like MATLAB
  UNITY_BEGIN();
  RUN_TEST(test_empty_address_gives_nack);
  RUN_TEST(test_slave_acks_azimuth);
  RUN_TEST(test_slave_acks_elevation);
  UNITY_END();
}

void loop() {}