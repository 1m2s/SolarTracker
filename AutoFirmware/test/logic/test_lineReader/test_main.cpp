#include <unity.h>
#include <string.h>
#include "LineReader.h"
#include "parseAngles.h"
 
// LineReader collects characters until '\n'.
// '\r' is skipped, empty lines are ignored, too long lines are thrown away.
 
// Helper: feed a whole text, give back the status of the LAST character
static LineStatus feedAll(LineReader &r, const char *s)
{
    LineStatus st = LINE_NONE;
    while (*s != '\0') {
        st = lineFeed(r, *s);
        s++;
    }
    return st;
}
 
// Helper: a text with n times 'x'
static void makeX(char *out, uint8_t n)
{
    memset(out, 'x', n);
    out[n] = '\0';
}
 
void setUp(void) {}
void tearDown(void) {}
 
void test_normal_line(void)
{
    LineReader r;
    lineClear(r);
    TEST_ASSERT_EQUAL(LINE_NONE, feedAll(r, "245.41 3.86"));   // not finished yet
    TEST_ASSERT_EQUAL(LINE_READY, lineFeed(r, '\n'));          // now finished
    TEST_ASSERT_EQUAL_STRING("245.41 3.86", r.buf);
}
 
void test_crlf(void)
{
    LineReader r;
    lineClear(r);
    TEST_ASSERT_EQUAL(LINE_READY, feedAll(r, "245.41 3.86\r\n"));
    TEST_ASSERT_EQUAL_STRING("245.41 3.86", r.buf);            // no '\r' inside
}
 
void test_two_lines(void)
{
    LineReader r;
    lineClear(r);
    TEST_ASSERT_EQUAL(LINE_READY, feedAll(r, "180 0\n"));
    TEST_ASSERT_EQUAL_STRING("180 0", r.buf);
    TEST_ASSERT_EQUAL(LINE_READY, feedAll(r, "250.1 -5.2\n"));
    TEST_ASSERT_EQUAL_STRING("250.1 -5.2", r.buf);
}
 
void test_exactly_31_is_ok(void)
{
    LineReader r;
    lineClear(r);
    char s[LINE_LEN_MAX + 1];
    makeX(s, LINE_LEN_MAX);                                    // 31 x 'x'
    feedAll(r, s);
    TEST_ASSERT_EQUAL(LINE_READY, lineFeed(r, '\n'));
    TEST_ASSERT_EQUAL_STRING(s, r.buf);
}
 
void test_32_is_too_long(void)
{
    LineReader r;
    lineClear(r);
    char s[LINE_LEN_MAX + 2];
    makeX(s, LINE_LEN_MAX + 1);                                // 32 x 'x'
    feedAll(r, s);
    TEST_ASSERT_EQUAL(LINE_TOO_LONG, lineFeed(r, '\n'));
}
 
void test_recovers_after_too_long(void)
{
    LineReader r;
    lineClear(r);
    char s[41];
    makeX(s, 40);                                              // 40 x 'x'
    feedAll(r, s);
    TEST_ASSERT_EQUAL(LINE_TOO_LONG, lineFeed(r, '\n'));
    TEST_ASSERT_EQUAL(LINE_READY, feedAll(r, "180 0\n"));      // next line is fine
    TEST_ASSERT_EQUAL_STRING("180 0", r.buf);
}
 
void test_empty_line_ignored(void)
{
    LineReader r;
    lineClear(r);
    TEST_ASSERT_EQUAL(LINE_NONE, lineFeed(r, '\n'));           // only Enter
    TEST_ASSERT_EQUAL(LINE_NONE, feedAll(r, "\r\n"));          // Enter with '\r'
    TEST_ASSERT_EQUAL(LINE_READY, feedAll(r, "180 0\n"));      // still works after
    TEST_ASSERT_EQUAL_STRING("180 0", r.buf);
}
 
// Half line "245" is thrown away. Without lineClear it would become "245180 0".
void test_clear_drops_half_line(void)
{
    LineReader r;
    lineClear(r);
    feedAll(r, "245");
    lineClear(r);
    TEST_ASSERT_EQUAL(LINE_READY, feedAll(r, "180 0\n"));
    TEST_ASSERT_EQUAL_STRING("180 0", r.buf);
}
 
// Together with the parser: a line typed in the monitor ("\r\n" at the end)
void test_ready_line_parses(void)
{
    LineReader r;
    lineClear(r);
    float a = 0.0f, g = 0.0f;
    TEST_ASSERT_EQUAL(LINE_READY, feedAll(r, "245.41 3.86\r\n"));
    TEST_ASSERT_TRUE(parseAngles(r.buf, a, g));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 245.41f, a);
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 3.86f, g);
}
 
int runAllTests(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_normal_line);
    RUN_TEST(test_crlf);
    RUN_TEST(test_two_lines);
    RUN_TEST(test_exactly_31_is_ok);
    RUN_TEST(test_32_is_too_long);
    RUN_TEST(test_recovers_after_too_long);
    RUN_TEST(test_empty_line_ignored);
    RUN_TEST(test_clear_drops_half_line);
    RUN_TEST(test_ready_line_parses);
    return UNITY_END();
}
 
#ifdef ARDUINO
#include <Arduino.h>
void setup()
{
    delay(2000);       // wait for the Serial monitor
    runAllTests();
}
void loop() {}
#else
int main(void)
{
    return runAllTests();
}
#endif