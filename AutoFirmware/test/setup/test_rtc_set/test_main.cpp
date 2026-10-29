/* Notes
   One-time tool. It sets the DS1302 to the PC compile time.
   Run it ONLY in winter time (MEZ).
   Each build sets the clock exactly once.
   It leaves a "stamp" (the build time) in the clock's battery RAM.
   After a reset it sees its own stamp and does not set again.
   After the tests, loop() prints the time every second.
   Compare it with your phone.
   Run: pio test -e nanosetup -f setup/test_rtc_set
 */

#include <Arduino.h>
#include <unity.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>

const uint8_t PIN_CLK = 5;
const uint8_t PIN_DAT = 6;
const uint8_t PIN_RST = 7;

ThreeWire rtcWire(PIN_DAT, PIN_CLK, PIN_RST);   // order: IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(rtcWire);

const RtcDateTime compiled(__DATE__, __TIME__);

void setUp(void) {}
void tearDown(void) {}

void printTime(const RtcDateTime &t) {
  char buf[20];
  snprintf(buf, sizeof(buf), "%04u-%02u-%02u %02u:%02u:%02u",
           t.Year(), t.Month(), t.Day(), t.Hour(), t.Minute(), t.Second());
  Serial.println(buf);
}

uint32_t readStamp() {
  uint32_t s = 0;
  for (uint8_t i = 0; i < 4; i++) {
    s |= (uint32_t)Rtc.GetMemory(i) << (8 * i);
  }
  return s;
}

void writeStamp(uint32_t s) {
  for (uint8_t i = 0; i < 4; i++) {
    Rtc.SetMemory(i, (uint8_t)(s >> (8 * i)));
  }
}

void test_rtc_unlocked_and_running(void) {
  if (Rtc.GetIsWriteProtected()) Rtc.SetIsWriteProtected(false);
  if (!Rtc.GetIsRunning()) Rtc.SetIsRunning(true);
  TEST_ASSERT_FALSE(Rtc.GetIsWriteProtected());
  TEST_ASSERT_TRUE(Rtc.GetIsRunning());
}

void test_rtc_set_once_per_build(void) {
  uint32_t buildStamp = compiled.TotalSeconds();
  if (readStamp() != buildStamp) {
    Rtc.SetDateTime(compiled);
    writeStamp(buildStamp);
  }
  TEST_ASSERT_EQUAL_UINT32(buildStamp, readStamp());
  RtcDateTime now = Rtc.GetDateTime();
  TEST_ASSERT_TRUE(now.IsValid());
  TEST_ASSERT_FALSE(now < compiled);
}

void setup() {
  delay(2000);
  Rtc.Begin();
  UNITY_BEGIN();
  RUN_TEST(test_rtc_unlocked_and_running);
  RUN_TEST(test_rtc_set_once_per_build);
  UNITY_END();
  Serial.begin(115200);
}

void loop() {
  printTime(Rtc.GetDateTime());
  delay(1000);
}