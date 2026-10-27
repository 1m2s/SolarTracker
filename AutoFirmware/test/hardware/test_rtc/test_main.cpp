/* Notes
   Hardware test for the DS1302 clock. It only READS. It never writes.
   Needs: clock wired to D5 (CLK), D6 (DAT), D7 (RST), USB to left Nano.
   Run: pio test -e nanotest -f hardware/test_rtc
   After the tests, loop() prints the time every second.
   test/hardware/test_rtc/check_rtc.py reads these lines and compares them with the Mac.
   Not for the PC. It is in test_ignore of [env:native]. */

#include <Arduino.h>
#include <unity.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>

const uint8_t PIN_CLK = 5;
const uint8_t PIN_DAT = 6;
const uint8_t PIN_RST = 7;

ThreeWire rtcWire(PIN_DAT, PIN_CLK, PIN_RST);   // order: IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(rtcWire);

void setUp(void) {}
void tearDown(void) {}

void test_rtc_is_running(void) {
  TEST_ASSERT_TRUE(Rtc.GetIsRunning());
}

void test_rtc_time_plausible(void) {
  RtcDateTime now = Rtc.GetDateTime();
  TEST_ASSERT_TRUE(now.IsValid());
  TEST_ASSERT_TRUE(now.Year() >= 2026);
  TEST_ASSERT_TRUE(now.Year() <= 2099);
}

void test_rtc_seconds_tick(void) {
  uint32_t t1 = Rtc.GetDateTime().TotalSeconds();
  delay(2000);
  uint32_t t2 = Rtc.GetDateTime().TotalSeconds();
  TEST_ASSERT_UINT32_WITHIN(1, 2, t2 - t1);
}

void setup() {
  delay(2000);
  Rtc.Begin();
  UNITY_BEGIN();
  RUN_TEST(test_rtc_is_running);
  RUN_TEST(test_rtc_time_plausible);
  RUN_TEST(test_rtc_seconds_tick);
  UNITY_END();
  Serial.begin(115200);
}

void loop() {
  RtcDateTime t = Rtc.GetDateTime();
  char buf[20];
  snprintf(buf, sizeof(buf), "%04u-%02u-%02u %02u:%02u:%02u",
           t.Year(), t.Month(), t.Day(), t.Hour(), t.Minute(), t.Second());
  Serial.println(buf);
  delay(1000);
}