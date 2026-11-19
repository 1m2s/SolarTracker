#ifndef READINGS_H
#define READINGS_H

#include <stdint.h>

const uint16_t READING_MAX      = 1023u;
const uint8_t  READINGS_PER_SUM = 16u;
const uint8_t  ANSWER_LENGTH    = 5u;

struct ReadingSums
{
    uint16_t sumA0;
    uint16_t sumA1;
    uint8_t  count;
};

struct Answer
{
    uint8_t bytes[ANSWER_LENGTH];
};

bool addReading(ReadingSums &sums, int rawA0, int rawA1, uint16_t &doneA0, uint16_t &doneA1);

void packAnswer(uint8_t status, uint16_t sumA0, uint16_t sumA1, Answer &answer);

#endif