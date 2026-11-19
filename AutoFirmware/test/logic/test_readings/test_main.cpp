#include <unity.h>
#include "Controller.h"
#include "config/Board.h"

static const bool SWITCH_OPEN = true;

static const bool ANSWERED  = true;
static const bool NO_ANSWER = false;

static const uint16_t ROOM_LIGHT_A0 = 2712u;
static const uint16_t NOISE_A1      = 19u;

static const uint16_t MISSING_BYTES_SUM = 0xFFFFu;

static const uint8_t OLD_STATUS_BITS = (uint8_t)(STATUS_IMU_ON | STATUS_MOVING);
static const uint8_t ALL_STATUS_BITS = (uint8_t)(STATUS_IMU_ON | STATUS_MOVING | STATUS_READINGS);

static Controller controller;

void setUp(void)
{
    ctrlInit(controller, SWITCH_OPEN, 0u);
}

void tearDown(void) {}

void test_sum_from_bytes_low_byte_first(void)
{
    TEST_ASSERT_EQUAL_UINT16(0u, ctrlSumFromBytes(0x00u, 0x00u));
    TEST_ASSERT_EQUAL_UINT16(255u, ctrlSumFromBytes(0xFFu, 0x00u));
    TEST_ASSERT_EQUAL_UINT16(256u, ctrlSumFromBytes(0x00u, 0x01u));
    TEST_ASSERT_EQUAL_UINT16(ROOM_LIGHT_A0, ctrlSumFromBytes(0x98u, 0x0Au));
    TEST_ASSERT_EQUAL_UINT16(READING_SUM_MAX, ctrlSumFromBytes(0xF0u, 0x3Fu));
    TEST_ASSERT_EQUAL_UINT16(MISSING_BYTES_SUM, ctrlSumFromBytes(0xFFu, 0xFFu));
}

void test_sum_max_is_16_full_readings(void)
{
    TEST_ASSERT_EQUAL_UINT16(16368u, READING_SUM_MAX);
}

void test_readings_unknown_after_init(void)
{
    TEST_ASSERT_FALSE(ctrlReadingsKnown(controller));
    TEST_ASSERT_EQUAL_UINT16(0u, controller.sumA0);
    TEST_ASSERT_EQUAL_UINT16(0u, controller.sumA1);
}

void test_good_answer_stores_both_sums(void)
{
    ctrlReadings(controller, ANSWERED, STATUS_READINGS, ROOM_LIGHT_A0, NOISE_A1);
    TEST_ASSERT_TRUE(ctrlReadingsKnown(controller));
    TEST_ASSERT_EQUAL_UINT16(ROOM_LIGHT_A0, controller.sumA0);
    TEST_ASSERT_EQUAL_UINT16(NOISE_A1, controller.sumA1);
}

void test_other_status_bits_do_not_matter(void)
{
    ctrlReadings(controller, ANSWERED, ALL_STATUS_BITS, ROOM_LIGHT_A0, NOISE_A1);
    TEST_ASSERT_TRUE(ctrlReadingsKnown(controller));
    TEST_ASSERT_EQUAL_UINT16(ROOM_LIGHT_A0, controller.sumA0);
}

void test_zero_sums_are_real_readings(void)
{
    ctrlReadings(controller, ANSWERED, STATUS_READINGS, 0u, 0u);
    TEST_ASSERT_TRUE(ctrlReadingsKnown(controller));
    TEST_ASSERT_EQUAL_UINT16(0u, controller.sumA0);
    TEST_ASSERT_EQUAL_UINT16(0u, controller.sumA1);
}

void test_largest_sums_are_accepted(void)
{
    ctrlReadings(controller, ANSWERED, STATUS_READINGS, READING_SUM_MAX, READING_SUM_MAX);
    TEST_ASSERT_TRUE(ctrlReadingsKnown(controller));
    TEST_ASSERT_EQUAL_UINT16(READING_SUM_MAX, controller.sumA0);
    TEST_ASSERT_EQUAL_UINT16(READING_SUM_MAX, controller.sumA1);
}

void test_a0_one_above_largest_is_dropped(void)
{
    ctrlReadings(controller, ANSWERED, STATUS_READINGS, (uint16_t)(READING_SUM_MAX + 1u), NOISE_A1);
    TEST_ASSERT_FALSE(ctrlReadingsKnown(controller));
}

void test_a1_one_above_largest_is_dropped(void)
{
    ctrlReadings(controller, ANSWERED, STATUS_READINGS, ROOM_LIGHT_A0, (uint16_t)(READING_SUM_MAX + 1u));
    TEST_ASSERT_FALSE(ctrlReadingsKnown(controller));
}

void test_sums_not_ready_are_dropped(void)
{
    ctrlReadings(controller, ANSWERED, OLD_STATUS_BITS, ROOM_LIGHT_A0, NOISE_A1);
    TEST_ASSERT_FALSE(ctrlReadingsKnown(controller));
}

void test_old_right_nano_is_dropped(void)
{
    ctrlReadings(controller, ANSWERED, STATUS_IMU_ON, MISSING_BYTES_SUM, MISSING_BYTES_SUM);
    TEST_ASSERT_FALSE(ctrlReadingsKnown(controller));
}

void test_all_ones_answer_is_dropped(void)
{
    ctrlReadings(controller, ANSWERED, 0xFFu, MISSING_BYTES_SUM, MISSING_BYTES_SUM);
    TEST_ASSERT_FALSE(ctrlReadingsKnown(controller));
}

void test_no_answer_is_dropped(void)
{
    ctrlReadings(controller, NO_ANSWER, STATUS_READINGS, ROOM_LIGHT_A0, NOISE_A1);
    TEST_ASSERT_FALSE(ctrlReadingsKnown(controller));
}

void test_bad_answer_clears_the_old_sums(void)
{
    ctrlReadings(controller, ANSWERED, STATUS_READINGS, ROOM_LIGHT_A0, NOISE_A1);
    ctrlReadings(controller, NO_ANSWER, 0u, 0u, 0u);
    TEST_ASSERT_FALSE(ctrlReadingsKnown(controller));
    TEST_ASSERT_EQUAL_UINT16(0u, controller.sumA0);
    TEST_ASSERT_EQUAL_UINT16(0u, controller.sumA1);
}

void test_good_answer_after_bad_one_is_used_again(void)
{
    ctrlReadings(controller, ANSWERED, STATUS_READINGS, ROOM_LIGHT_A0, NOISE_A1);
    ctrlReadings(controller, NO_ANSWER, 0u, 0u, 0u);
    ctrlReadings(controller, ANSWERED, STATUS_READINGS, 5184u, 8u);
    TEST_ASSERT_TRUE(ctrlReadingsKnown(controller));
    TEST_ASSERT_EQUAL_UINT16(5184u, controller.sumA0);
    TEST_ASSERT_EQUAL_UINT16(8u, controller.sumA1);
}

void test_new_sums_replace_the_old_ones(void)
{
    ctrlReadings(controller, ANSWERED, STATUS_READINGS, ROOM_LIGHT_A0, NOISE_A1);
    ctrlReadings(controller, ANSWERED, STATUS_READINGS, 3878u, 29u);
    TEST_ASSERT_EQUAL_UINT16(3878u, controller.sumA0);
    TEST_ASSERT_EQUAL_UINT16(29u, controller.sumA1);
}

void test_readings_hidden_after_a_fault(void)
{
    TEST_ASSERT_EQUAL(RESULT_LATCHED, ctrlStatus(controller, true, STATUS_READINGS, 0u));
    ctrlReadings(controller, ANSWERED, STATUS_READINGS, ROOM_LIGHT_A0, NOISE_A1);
    TEST_ASSERT_TRUE(ctrlReadingsKnown(controller));

    uint8_t switchMoved = (uint8_t)(STATUS_IMU_ON | STATUS_READINGS);
    TEST_ASSERT_EQUAL(RESULT_FAULT, ctrlStatus(controller, true, switchMoved, COMPASS_CHECK_MS));
    ctrlReadings(controller, ANSWERED, switchMoved, ROOM_LIGHT_A0, NOISE_A1);
    TEST_ASSERT_FALSE(ctrlReadingsKnown(controller));
}

void test_readings_bit_does_not_disturb_the_switch(void)
{
    TEST_ASSERT_EQUAL(RESULT_LATCHED, ctrlStatus(controller, true, STATUS_IMU_ON, 0u));
    uint8_t withReadings = (uint8_t)(STATUS_IMU_ON | STATUS_READINGS);
    TEST_ASSERT_EQUAL(RESULT_SAME, ctrlStatus(controller, true, withReadings, COMPASS_CHECK_MS));
    TEST_ASSERT_EQUAL(FAULT_NONE, controller.fault);
    TEST_ASSERT_TRUE(controller.imuSwitchOn);
}

void test_init_forgets_the_readings(void)
{
    ctrlReadings(controller, ANSWERED, STATUS_READINGS, ROOM_LIGHT_A0, NOISE_A1);
    ctrlInit(controller, SWITCH_OPEN, 0u);
    TEST_ASSERT_FALSE(ctrlReadingsKnown(controller));
    TEST_ASSERT_EQUAL_UINT16(0u, controller.sumA0);
    TEST_ASSERT_EQUAL_UINT16(0u, controller.sumA1);
}

int runAllTests(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_sum_from_bytes_low_byte_first);
    RUN_TEST(test_sum_max_is_16_full_readings);
    RUN_TEST(test_readings_unknown_after_init);
    RUN_TEST(test_good_answer_stores_both_sums);
    RUN_TEST(test_other_status_bits_do_not_matter);
    RUN_TEST(test_zero_sums_are_real_readings);
    RUN_TEST(test_largest_sums_are_accepted);
    RUN_TEST(test_a0_one_above_largest_is_dropped);
    RUN_TEST(test_a1_one_above_largest_is_dropped);
    RUN_TEST(test_sums_not_ready_are_dropped);
    RUN_TEST(test_old_right_nano_is_dropped);
    RUN_TEST(test_all_ones_answer_is_dropped);
    RUN_TEST(test_no_answer_is_dropped);
    RUN_TEST(test_bad_answer_clears_the_old_sums);
    RUN_TEST(test_good_answer_after_bad_one_is_used_again);
    RUN_TEST(test_new_sums_replace_the_old_ones);
    RUN_TEST(test_readings_hidden_after_a_fault);
    RUN_TEST(test_readings_bit_does_not_disturb_the_switch);
    RUN_TEST(test_init_forgets_the_readings);
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