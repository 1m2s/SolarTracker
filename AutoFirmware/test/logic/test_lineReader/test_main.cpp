#include <unity.h>
#include <string.h>
#include "MatlabLink.h"
 
// LineReader collects characters until '\n'.
// '\r' is skipped, empty lines are ignored, too long lines are thrown away.
 
// Helper: feed a whole text, give back the status of the LAST character
static LineStatus feedAll(LineReader &reader, const char *text)
{
    LineStatus lineStatus = LINE_NONE;
    while (*text != '\0') {
        lineStatus = lineFeed(reader, *text);
        text++;
    }
    return lineStatus;
}
 
// Helper: a text with count times 'x'
static void makeX(char *out, uint8_t count)
{
    memset(out, 'x', count);
    out[count] = '\0';
}
 
void setUp(void) {}
void tearDown(void) {}
 
void test_normal_line(void)
{
    LineReader reader;
    lineClear(reader);
    TEST_ASSERT_EQUAL(LINE_NONE, feedAll(reader, "245.41 3.86"));   // not finished yet
    TEST_ASSERT_EQUAL(LINE_READY, lineFeed(reader, '\n'));          // now finished
    TEST_ASSERT_EQUAL_STRING("245.41 3.86", reader.buf);
}
 
void test_crlf(void)
{
    LineReader reader;
    lineClear(reader);
    TEST_ASSERT_EQUAL(LINE_READY, feedAll(reader, "245.41 3.86\r\n"));
    TEST_ASSERT_EQUAL_STRING("245.41 3.86", reader.buf);            // no '\r' inside
}
 
void test_two_lines(void)
{
    LineReader reader;
    lineClear(reader);
    TEST_ASSERT_EQUAL(LINE_READY, feedAll(reader, "180 0\n"));
    TEST_ASSERT_EQUAL_STRING("180 0", reader.buf);
    TEST_ASSERT_EQUAL(LINE_READY, feedAll(reader, "250.1 -5.2\n"));
    TEST_ASSERT_EQUAL_STRING("250.1 -5.2", reader.buf);
}
 
void test_exactly_31_is_ok(void)
{
    LineReader reader;
    lineClear(reader);
    char text[LINE_LEN_MAX + 1];
    makeX(text, LINE_LEN_MAX);                                      // 31 x 'x'
    feedAll(reader, text);
    TEST_ASSERT_EQUAL(LINE_READY, lineFeed(reader, '\n'));
    TEST_ASSERT_EQUAL_STRING(text, reader.buf);
}
 
void test_32_is_too_long(void)
{
    LineReader reader;
    lineClear(reader);
    char text[LINE_LEN_MAX + 2];
    makeX(text, LINE_LEN_MAX + 1);                                  // 32 x 'x'
    feedAll(reader, text);
    TEST_ASSERT_EQUAL(LINE_TOO_LONG, lineFeed(reader, '\n'));
}
 
void test_recovers_after_too_long(void)
{
    LineReader reader;
    lineClear(reader);
    char text[41];
    makeX(text, 40);                                                // 40 x 'x'
    feedAll(reader, text);
    TEST_ASSERT_EQUAL(LINE_TOO_LONG, lineFeed(reader, '\n'));
    TEST_ASSERT_EQUAL(LINE_READY, feedAll(reader, "180 0\n"));      // next line is fine
    TEST_ASSERT_EQUAL_STRING("180 0", reader.buf);
}
 
void test_empty_line_ignored(void)
{
    LineReader reader;
    lineClear(reader);
    TEST_ASSERT_EQUAL(LINE_NONE, lineFeed(reader, '\n'));           // only Enter
    TEST_ASSERT_EQUAL(LINE_NONE, feedAll(reader, "\r\n"));          // Enter with '\r'
    TEST_ASSERT_EQUAL(LINE_READY, feedAll(reader, "180 0\n"));      // still works after
    TEST_ASSERT_EQUAL_STRING("180 0", reader.buf);
}
 
// Half line "245" is thrown away. Without lineClear it would become "245180 0".
void test_clear_drops_half_line(void)
{
    LineReader reader;
    lineClear(reader);
    feedAll(reader, "245");
    lineClear(reader);
    TEST_ASSERT_EQUAL(LINE_READY, feedAll(reader, "180 0\n"));
    TEST_ASSERT_EQUAL_STRING("180 0", reader.buf);
}
 
// Together with the parser: a line typed in the monitor ("\r\n" at the end)
void test_ready_line_parses(void)
{
    LineReader reader;
    lineClear(reader);
    float azimuthDeg = 0.0f, elevationDeg = 0.0f;
    TEST_ASSERT_EQUAL(LINE_READY, feedAll(reader, "245.41 3.86\r\n"));
    TEST_ASSERT_TRUE(parseAngles(reader.buf, azimuthDeg, elevationDeg));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 245.41f, azimuthDeg);
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 3.86f, elevationDeg);
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