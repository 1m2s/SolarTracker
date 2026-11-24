#include <unity.h>
#include <stdint.h>
#include "Readings.h"

void setUp(void) {}
void tearDown(void) {}

static uint8_t addMany(ReadingSums &sums, int rawA0, int rawA1, uint8_t times,
                       uint16_t &doneA0, uint16_t &doneA1)
{
    uint8_t finished = 0u;
    for (uint8_t i = 0u; i < times; i++) {
        if (addReading(sums, rawA0, rawA1, doneA0, doneA1)) {
            finished++;
        }
    }
    return finished;
}

void test_first_15_readings_finish_nothing(void)
{
    ReadingSums sums = {0u, 0u, 0u};
    uint16_t doneA0 = 7u;
    uint16_t doneA1 = 9u;
    TEST_ASSERT_EQUAL_UINT8(0u, addMany(sums, 417, 74, 15u, doneA0, doneA1));
    TEST_ASSERT_EQUAL_UINT16(7u, doneA0);
    TEST_ASSERT_EQUAL_UINT16(9u, doneA1);
    TEST_ASSERT_EQUAL_UINT8(15u, sums.count);
}

void test_16th_reading_finishes_the_sums(void)
{
    ReadingSums sums = {0u, 0u, 0u};
    uint16_t doneA0 = 0u;
    uint16_t doneA1 = 0u;
    TEST_ASSERT_EQUAL_UINT8(1u, addMany(sums, 417, 74, 16u, doneA0, doneA1));
    TEST_ASSERT_EQUAL_UINT16(6672u, doneA0);
    TEST_ASSERT_EQUAL_UINT16(1184u, doneA1);
}

void test_sums_start_again_after_a_finished_sum(void)
{
    ReadingSums sums = {0u, 0u, 0u};
    uint16_t doneA0 = 0u;
    uint16_t doneA1 = 0u;
    addMany(sums, 1000, 500, 16u, doneA0, doneA1);
    TEST_ASSERT_EQUAL_UINT16(0u, sums.sumA0);
    TEST_ASSERT_EQUAL_UINT16(0u, sums.sumA1);
    TEST_ASSERT_EQUAL_UINT8(0u, sums.count);

    TEST_ASSERT_EQUAL_UINT8(1u, addMany(sums, 3, 1, 16u, doneA0, doneA1));
    TEST_ASSERT_EQUAL_UINT16(48u, doneA0);
    TEST_ASSERT_EQUAL_UINT16(16u, doneA1);
}

void test_different_readings_are_added(void)
{
    ReadingSums sums = {0u, 0u, 0u};
    uint16_t doneA0 = 0u;
    uint16_t doneA1 = 0u;
    bool finished = false;
    for (int i = 1; i <= 16; i++) {
        finished = addReading(sums, i, 2 * i, doneA0, doneA1);
    }
    TEST_ASSERT_TRUE(finished);
    TEST_ASSERT_EQUAL_UINT16(136u, doneA0);
    TEST_ASSERT_EQUAL_UINT16(272u, doneA1);
}

void test_all_zero_gives_zero(void)
{
    ReadingSums sums = {0u, 0u, 0u};
    uint16_t doneA0 = 5u;
    uint16_t doneA1 = 5u;
    TEST_ASSERT_EQUAL_UINT8(1u, addMany(sums, 0, 0, 16u, doneA0, doneA1));
    TEST_ASSERT_EQUAL_UINT16(0u, doneA0);
    TEST_ASSERT_EQUAL_UINT16(0u, doneA1);
}

void test_largest_readings_give_16368(void)
{
    ReadingSums sums = {0u, 0u, 0u};
    uint16_t doneA0 = 0u;
    uint16_t doneA1 = 0u;
    addMany(sums, 1023, 1023, 16u, doneA0, doneA1);
    TEST_ASSERT_EQUAL_UINT16(16368u, doneA0);
    TEST_ASSERT_EQUAL_UINT16(16368u, doneA1);
}

void test_readings_outside_the_range_are_limited(void)
{
    ReadingSums sums = {0u, 0u, 0u};
    uint16_t doneA0 = 1u;
    uint16_t doneA1 = 1u;
    addMany(sums, -5, 1024, 16u, doneA0, doneA1);
    TEST_ASSERT_EQUAL_UINT16(0u, doneA0);
    TEST_ASSERT_EQUAL_UINT16(16368u, doneA1);

    addMany(sums, 32767, -32767, 16u, doneA0, doneA1);
    TEST_ASSERT_EQUAL_UINT16(16368u, doneA0);
    TEST_ASSERT_EQUAL_UINT16(0u, doneA1);
}

void test_full_count_starts_again(void)
{
    ReadingSums sums = {60000u, 60000u, 200u};
    uint16_t doneA0 = 0u;
    uint16_t doneA1 = 0u;
    TEST_ASSERT_EQUAL_UINT8(1u, addMany(sums, 1023, 1023, 16u, doneA0, doneA1));
    TEST_ASSERT_EQUAL_UINT16(16368u, doneA0);
    TEST_ASSERT_EQUAL_UINT16(16368u, doneA1);
}

void test_many_sums_in_a_row(void)
{
    ReadingSums sums = {0u, 0u, 0u};
    uint16_t doneA0 = 0u;
    uint16_t doneA1 = 0u;
    TEST_ASSERT_EQUAL_UINT8(10u, addMany(sums, 100, 10, 160u, doneA0, doneA1));
    TEST_ASSERT_EQUAL_UINT16(1600u, doneA0);
    TEST_ASSERT_EQUAL_UINT16(160u, doneA1);
}

void test_pack_puts_the_low_byte_first(void)
{
    Answer answer;
    packAnswer(0x05u, 0x1A2Bu, 0x3C4Du, answer);
    TEST_ASSERT_EQUAL_HEX8(0x05u, answer.bytes[0]);
    TEST_ASSERT_EQUAL_HEX8(0x2Bu, answer.bytes[1]);
    TEST_ASSERT_EQUAL_HEX8(0x1Au, answer.bytes[2]);
    TEST_ASSERT_EQUAL_HEX8(0x4Du, answer.bytes[3]);
    TEST_ASSERT_EQUAL_HEX8(0x3Cu, answer.bytes[4]);
}

void test_pack_zero_and_largest_sum(void)
{
    Answer answer;
    packAnswer(0x00u, 0u, 16368u, answer);
    TEST_ASSERT_EQUAL_HEX8(0x00u, answer.bytes[0]);
    TEST_ASSERT_EQUAL_HEX8(0x00u, answer.bytes[1]);
    TEST_ASSERT_EQUAL_HEX8(0x00u, answer.bytes[2]);
    TEST_ASSERT_EQUAL_HEX8(0xF0u, answer.bytes[3]);
    TEST_ASSERT_EQUAL_HEX8(0x3Fu, answer.bytes[4]);
}

void test_packed_sum_can_be_read_back(void)
{
    Answer answer;
    packAnswer(0x07u, 6672u, 1184u, answer);
    uint16_t backA0 = (uint16_t)(answer.bytes[1] | ((uint16_t)answer.bytes[2] << 8));
    uint16_t backA1 = (uint16_t)(answer.bytes[3] | ((uint16_t)answer.bytes[4] << 8));
    TEST_ASSERT_EQUAL_UINT16(6672u, backA0);
    TEST_ASSERT_EQUAL_UINT16(1184u, backA1);
}

int runAllTests(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_first_15_readings_finish_nothing);
    RUN_TEST(test_16th_reading_finishes_the_sums);
    RUN_TEST(test_sums_start_again_after_a_finished_sum);
    RUN_TEST(test_different_readings_are_added);
    RUN_TEST(test_all_zero_gives_zero);
    RUN_TEST(test_largest_readings_give_16368);
    RUN_TEST(test_readings_outside_the_range_are_limited);
    RUN_TEST(test_full_count_starts_again);
    RUN_TEST(test_many_sums_in_a_row);
    RUN_TEST(test_pack_puts_the_low_byte_first);
    RUN_TEST(test_pack_zero_and_largest_sum);
    RUN_TEST(test_packed_sum_can_be_read_back);
    return UNITY_END();
}

#ifdef ARDUINO
#include <Arduino.h>
void setup()
{
    delay(2000);
    runAllTests();
}
void loop() {}
#else
int main(void)
{
    return runAllTests();
}
#endif