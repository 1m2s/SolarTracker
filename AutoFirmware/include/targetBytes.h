#ifndef TARGETBYTES_H
#define TARGETBYTES_H

#include <stdint.h>

// Turns the sun position into the two bytes for the right Nano.
// Night or unsafe value -> park position (south, vertical).
// Gives back true when parked.
bool targetBytes(float alpha, float gamma, uint8_t &a8, uint8_t &g8);

#endif