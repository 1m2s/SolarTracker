/* Notes
   Hardware test: knocks on every I2C address and lists who answers.
   It only READS: no data is sent, nothing moves.
   Question it answers: does the compass (0x0C) only show up
   when the IMU switch is AN?
   Each address gets 5 knocks, because in IMU mode the right Nano
   also uses the bus and a knock can collide with it.
   Needs: fresh power-on, wait 5 s, USB to left Nano.
   Run: pio test -e nanotest -f hardware/test_i2c_fullscan
*/

#include <Arduino.h>
#include <Wire.h>
#include <unity.h>

const uint8_t FIRST_ADDR = 0x08;   // normal 7-bit address range
const uint8_t LAST_ADDR  = 0x77;
const uint8_t KNOCKS     = 5;      // knocks per address

uint8_t acks[128];                 // how many knocks each address answered

// Knocks once. 0 = answered, 2 = nobody there, other = bus problem
uint8_t knock(uint8_t addr)
{
    Wire.beginTransmission(addr);
    return Wire.endTransmission();
}

void setUp(void) {}
void tearDown(void) {}

void test_scan_bus(void)
{
    char line[48];
    uint8_t problems = 0;          // knocks that ended with a bus problem

    for (uint8_t addr = FIRST_ADDR; addr <= LAST_ADDR; addr++) {
        acks[addr] = 0;
        for (uint8_t i = 0; i < KNOCKS; i++) {
            uint8_t code = knock(addr);
            if (code == 0) {
                acks[addr]++;
            } else if (code != 2) {
                problems++;
            }
        }
        if (acks[addr] > 0) {
            snprintf(line, sizeof(line), "found 0x%02X  (%u/%u knocks)",
                     addr, acks[addr], KNOCKS);
            TEST_MESSAGE(line);
        }
    }

    snprintf(line, sizeof(line), "bus problems: %u", problems);
    TEST_MESSAGE(line);
}

// Proves the scan works at all.
void test_right_nano_found(void)
{
    TEST_ASSERT_TRUE_MESSAGE(acks[0x19] > 0, "right Nano (0x19) not found");
}

// The experiment. No pass/fail yet, we only look.
void test_report_compass(void)
{
    TEST_MESSAGE(acks[0x0C] > 0 ? "compass 0x0C: ANSWERS"
                                : "compass 0x0C: silent");
}

void setup()
{
    delay(2000);                   // wait for the Serial monitor
    Wire.begin();                  // this Nano is the I2C master
    Wire.setWireTimeout(3000, true);   // 3 ms: never hang on a stuck bus
    UNITY_BEGIN();
    RUN_TEST(test_scan_bus);
    RUN_TEST(test_right_nano_found);
    RUN_TEST(test_report_compass);
    UNITY_END();
}

void loop() {}


/* Result, 2 Nov 2026 (thesis firmware on the right Nano)
   IMU switch AUS: 0x19, 0x68, 0x77 answered 5/5, no bus problems.
   IMU switch AN:  nothing answered, 48 bus problems (two runs).
   -> In "mit IMU" mode the right Nano also uses the bus as master,
      and the whole bus stops working for the left Nano.
   0x0C (compass) never answered.
   This is why Phase 4 moves the compass to the left Nano.
*/