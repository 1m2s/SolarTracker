/**
 * @file MatlabLink.h
 * @brief Read the angle lines that MATLAB sends over USB
 *
 * In MATLAB mode, AusgabeSonnenstandSerial.m sends one line per target, e.g.
 * "245.41 3.86\n" (azimuth elevation, in deg). USB delivers it one character
 * at a time: lineFeed collects the characters into a line, parseAngles turns
 * the finished line into two checked angles.
 *
 * Target: Arduino Nano (ATmega328P) and PC (host tests). Level: L2.
 *
 * @author Mohamed Sellami
 * @date 2026-11-03
 *
 */
#ifndef MATLABLINK_H
#define MATLABLINK_H

#include <stdint.h>

/******************************************************************************
 Constants
******************************************************************************/
/*
 * Longest line accepted, in characters, without the end marker.
 * Not called LINE_MAX: Linux already defines that name, which clashes on the PC.
 */
const uint8_t LINE_LEN_MAX = 31;

/******************************************************************************
 Types
******************************************************************************/
/** @brief What lineFeed reports after each character */
enum LineStatus : uint8_t {
    LINE_NONE,        /**< Line not finished yet, or an empty line */
    LINE_READY,       /**< A full line is now in buf */
    LINE_TOO_LONG     /**< The line was longer than LINE_LEN_MAX and was thrown away */
};

/**
 * @brief Collects USB characters until '\n'
 *
 * Call lineClear once before the first use.
 */
struct LineReader {
    char    buf[LINE_LEN_MAX + 1];   /**< The line plus the end marker '\0' */
    uint8_t len;                     /**< Characters stored so far, 0..LINE_LEN_MAX */
    bool    tooLong;                 /**< true = more than LINE_LEN_MAX characters came */
};

/******************************************************************************
 Function Declarations
******************************************************************************/
/**
 * @brief Feed one character into the line reader
 *
 * '\r' is skipped, so "\r\n" endings work. A '\n' ends the line. Characters
 * beyond LINE_LEN_MAX are not stored; the reader waits for '\n', reports
 * LINE_TOO_LONG and starts fresh, so a long line never overruns buf and
 * never sticks to the next line.
 *
 * @param[in,out] reader     Reader state.
 * @param[in]     character  Character from USB.
 *
 * @return LINE_READY when a full line is in reader.buf, LINE_TOO_LONG when a
 *         too long line was thrown away, LINE_NONE otherwise (also for an
 *         empty line).
 *
 * @note After LINE_READY, use reader.buf right away: the next character
 *       starts writing over it.
 */
LineStatus lineFeed(LineReader &reader, char character);

/**
 * @brief Throw away a half line and start fresh
 *
 * @param[out] reader  Reader state, emptied.
 */
void lineClear(LineReader &reader);

/**
 * @brief Read "azimuth elevation" from one MATLAB line
 *
 * Given a line like "245.41 3.86", reads two numbers separated by blanks.
 * Blanks after the second number are allowed, anything else is not.
 *
 * @param[in]  line          Text ending in '\0'. Must not be NULL.
 * @param[out] azimuthDeg    Azimuth, deg, 0..360. Written only if the line is
 *                           OK.
 * @param[out] elevationDeg  Elevation, deg, -90..90. Written only if the line
 *                           is OK.
 *
 * @return true if the line is OK, false if it is broken or out of range
 *         (then azimuthDeg and elevationDeg keep their old values).
 *
 * @note Night values like "250.1 -5.2" are OK here; targetBytes parks them
 *       later, the same as in AUTO.
 */
bool parseAngles(const char *line, float &azimuthDeg, float &elevationDeg);

#endif /* MATLABLINK_H */