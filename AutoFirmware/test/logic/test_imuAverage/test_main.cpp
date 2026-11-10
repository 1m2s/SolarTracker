#include <unity.h>
#include <stdint.h>
#include "ImuAverage.h"

void setUp(void) {}
void tearDown(void) {}

ImuRaw makeRaw(int16_t accelX, int16_t accelY, int16_t magX, int16_t magY, int16_t magZ)
{
    ImuRaw raw;
    raw.accelX = accelX;
    raw.accelY = accelY;
    raw.magX   = magX;
    raw.magY   = magY;
    raw.magZ   = magZ;
    return raw;
}

ImuRaw sameEverywhere(int16_t value)
{
    return makeRaw(value, value, value, value, value);
}

void assertRaw(const ImuRaw &expected, const ImuRaw &actual)
{
    TEST_ASSERT_EQUAL_INT16(expected.accelX, actual.accelX);
    TEST_ASSERT_EQUAL_INT16(expected.accelY, actual.accelY);
    TEST_ASSERT_EQUAL_INT16(expected.magX,   actual.magX);
    TEST_ASSERT_EQUAL_INT16(expected.magY,   actual.magY);
    TEST_ASSERT_EQUAL_INT16(expected.magZ,   actual.magZ);
}

void test_nothing_added_gives_no_result(void)
{
    ImuAverage average;
    averageReset(average);
    ImuRaw result = sameEverywhere(7);
    TEST_ASSERT_FALSE(averageResult(average, result));
    assertRaw(sameEverywhere(7), result);
}

void test_three_good_reads(void)
{
    ImuAverage average;
    averageReset(average);
    averageAdd(average, true, makeRaw(990, 340, -22, 84, 115));
    averageAdd(average, true, makeRaw(996, 350, -24, 86, 116));
    averageAdd(average, true, makeRaw(993, 357, -23, 85, 117));
    ImuRaw result;
    TEST_ASSERT_TRUE(averageResult(average, result));
    TEST_ASSERT_EQUAL_UINT16(3u, average.goodReads);
    assertRaw(makeRaw(993, 349, -23, 85, 116), result);
}

void test_bad_reads_are_skipped(void)
{
    ImuAverage average;
    averageReset(average);
    averageAdd(average, true,  makeRaw(100, 10, -20, 80, 110));
    averageAdd(average, false, sameEverywhere(32767));
    averageAdd(average, true,  makeRaw(102, 12, -22, 82, 112));
    averageAdd(average, false, sameEverywhere(0));
    ImuRaw result;
    TEST_ASSERT_TRUE(averageResult(average, result));
    TEST_ASSERT_EQUAL_UINT16(2u, average.goodReads);
    assertRaw(makeRaw(101, 11, -21, 81, 111), result);
}

void test_only_bad_reads_gives_no_result(void)
{
    ImuAverage average;
    averageReset(average);
    for (int readIndex = 0; readIndex < 100; readIndex++) {
        averageAdd(average, false, sameEverywhere(50));
    }
    ImuRaw result;
    TEST_ASSERT_FALSE(averageResult(average, result));
    TEST_ASSERT_EQUAL_UINT16(0u, average.goodReads);
}

void test_rounds_to_nearest(void)
{
    ImuAverage average;
    averageReset(average);
    averageAdd(average, true, makeRaw( 1, -1,  0,  0,  1));
    averageAdd(average, true, makeRaw( 2, -2,  0,  1,  1));
    averageAdd(average, true, makeRaw( 1, -1,  1,  1,  2));
    ImuRaw result;
    TEST_ASSERT_TRUE(averageResult(average, result));
    assertRaw(makeRaw(1, -1, 0, 1, 1), result);

    averageReset(average);
    averageAdd(average, true, makeRaw( 1, -1, 0, 0, 0));
    averageAdd(average, true, makeRaw( 2, -2, 0, 0, 0));
    TEST_ASSERT_TRUE(averageResult(average, result));
    TEST_ASSERT_EQUAL_INT16( 2, result.accelX);
    TEST_ASSERT_EQUAL_INT16(-2, result.accelY);
}

void test_extremes_100_reads(void)
{
    ImuAverage average;
    ImuRaw result;

    averageReset(average);
    for (int readIndex = 0; readIndex < 100; readIndex++) {
        averageAdd(average, true, sameEverywhere(32767));
    }
    TEST_ASSERT_TRUE(averageResult(average, result));
    assertRaw(sameEverywhere(32767), result);

    averageReset(average);
    for (int readIndex = 0; readIndex < 100; readIndex++) {
        averageAdd(average, true, sameEverywhere(-32768));
    }
    TEST_ASSERT_TRUE(averageResult(average, result));
    assertRaw(sameEverywhere(-32768), result);
}

void test_counter_stops_at_65535(void)
{
    ImuAverage average;
    ImuRaw result;

    averageReset(average);
    for (int32_t readIndex = 0; readIndex < 70000; readIndex++) {
        averageAdd(average, true, sameEverywhere(32767));
    }
    TEST_ASSERT_EQUAL_UINT16(65535u, average.goodReads);
    TEST_ASSERT_TRUE(averageResult(average, result));
    assertRaw(sameEverywhere(32767), result);

    averageReset(average);
    for (int32_t readIndex = 0; readIndex < 70000; readIndex++) {
        averageAdd(average, true, sameEverywhere(-32768));
    }
    TEST_ASSERT_EQUAL_UINT16(65535u, average.goodReads);
    TEST_ASSERT_TRUE(averageResult(average, result));
    assertRaw(sameEverywhere(-32768), result);
}

void test_reset_clears_everything(void)
{
    ImuAverage average;
    averageReset(average);
    averageAdd(average, true, makeRaw(500, 400, 300, 200, 100));
    averageReset(average);
    TEST_ASSERT_EQUAL_UINT16(0u, average.goodReads);
    TEST_ASSERT_EQUAL_INT32(0, average.sumAccelX);
    TEST_ASSERT_EQUAL_INT32(0, average.sumAccelY);
    TEST_ASSERT_EQUAL_INT32(0, average.sumMagX);
    TEST_ASSERT_EQUAL_INT32(0, average.sumMagY);
    TEST_ASSERT_EQUAL_INT32(0, average.sumMagZ);
    ImuRaw result;
    TEST_ASSERT_FALSE(averageResult(average, result));
}

int runAllTests(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_nothing_added_gives_no_result);
    RUN_TEST(test_three_good_reads);
    RUN_TEST(test_bad_reads_are_skipped);
    RUN_TEST(test_only_bad_reads_gives_no_result);
    RUN_TEST(test_rounds_to_nearest);
    RUN_TEST(test_extremes_100_reads);
    RUN_TEST(test_counter_stops_at_65535);
    RUN_TEST(test_reset_clears_everything);
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