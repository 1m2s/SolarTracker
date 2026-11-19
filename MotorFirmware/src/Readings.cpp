#include "Readings.h"

static uint16_t limitReading(int raw)
{
    if (raw < 0) {
        return 0u;
    }
    if (raw > (int)READING_MAX) {
        return READING_MAX;
    }
    return (uint16_t)raw;
}

bool addReading(ReadingSums &sums, int rawA0, int rawA1, uint16_t &doneA0, uint16_t &doneA1)
{
    if (sums.count >= READINGS_PER_SUM) {
        sums.sumA0 = 0u;
        sums.sumA1 = 0u;
        sums.count = 0u;
    }

    sums.sumA0 = (uint16_t)(sums.sumA0 + limitReading(rawA0));
    sums.sumA1 = (uint16_t)(sums.sumA1 + limitReading(rawA1));
    sums.count = (uint8_t)(sums.count + 1u);

    if (sums.count < READINGS_PER_SUM) {
        return false;
    }

    doneA0     = sums.sumA0;
    doneA1     = sums.sumA1;
    sums.sumA0 = 0u;
    sums.sumA1 = 0u;
    sums.count = 0u;
    return true;
}

void packAnswer(uint8_t status, uint16_t sumA0, uint16_t sumA1, Answer &answer)
{
    answer.bytes[0] = status;
    answer.bytes[1] = (uint8_t)(sumA0 & 0xFFu);
    answer.bytes[2] = (uint8_t)(sumA0 >> 8);
    answer.bytes[3] = (uint8_t)(sumA1 & 0xFFu);
    answer.bytes[4] = (uint8_t)(sumA1 >> 8);
}