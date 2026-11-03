/**
 * @file Controller.h
 * @brief Mode and timing decisions of AutoFirmware, without any hardware
 *
 * Decides which mode the firmware is in (debounced switch), when an AUTO
 * cycle is due, when MATLAB has timed out, whether there is an error, and
 * whether the LED is on. main.cpp reads the hardware, passes the facts in,
 * and acts on the answers. Because nothing here touches the hardware, all
 * of it is tested on the PC (test_controller).
 *
 * Call ctrlInit once before any other ctrl function.
 *
 * Target: Arduino Nano (ATmega328P) and PC (host tests). Level: L2.
 *
 * @author Mohamed Sellami
 * @date 2026-11-03
 *
 */
#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <stdint.h>
#include "config/Settings.h"

/******************************************************************************
 Types
******************************************************************************/
/**
 * @brief State kept between loop() passes
 *
 * Callers may read the fields (e.g. mode), but only the ctrl functions
 * write them. All times are millis() values.
 */
struct Controller {
    Mode     mode;              /**< Mode the firmware acts on, after debounce */
    bool     error;             /**< true = the last cycle failed, the LED blinks fast */
    bool     runNow;            /**< true = run an AUTO cycle now instead of waiting */
    uint32_t lastCycleMs;       /**< millis() of the last AUTO cycle */
    bool     lastSwitchOpen;    /**< Last raw switch reading, true = open (HIGH) */
    uint32_t switchChangedMs;   /**< millis() when that raw reading last changed */
    uint32_t lastMatlabMs;      /**< millis() of the last good MATLAB line */
    bool     timedOut;          /**< true = the MATLAB timeout was already reported */
};

/******************************************************************************
 Function Declarations
******************************************************************************/
/**
 * @brief Set the start state at power-on
 *
 * Takes the mode straight from the switch, without debounce: at power-on
 * there is no old mode to bounce from. In AUTO the first cycle runs at once,
 * in MATLAB the timeout starts now.
 *
 * @param[out] controller  State to set up.
 * @param[in]  switchOpen  Switch reading, true = open (HIGH) = AUTO.
 * @param[in]  nowMs       millis() now.
 */
void ctrlInit(Controller &controller, bool switchOpen, uint32_t nowMs);

/**
 * @brief Read the switch and accept a new mode after debounce
 *
 * A new switch position counts only after it stayed unchanged for
 * DEBOUNCE_MS. On a mode change, the error is cleared (it belonged to the
 * other mode); going to AUTO runs a cycle at once, going to MATLAB restarts
 * the timeout.
 *
 * @param[in,out] controller  State.
 * @param[in]     switchOpen  Switch reading now, true = open (HIGH) = AUTO.
 * @param[in]     nowMs       millis() now.
 *
 * @return true if the mode changed in this call (the caller then throws
 *         away old USB input and prints the new mode), false otherwise.
 */
bool ctrlSwitch(Controller &controller, bool switchOpen, uint32_t nowMs);

/**
 * @brief Check if an AUTO cycle should run now
 *
 * Due right after ctrlInit or a switch to AUTO, then every AUTO_INTERVAL_MS.
 * Never due in MATLAB mode. When due, the cycle is counted as started.
 *
 * @param[in,out] controller  State.
 * @param[in]     nowMs       millis() now.
 *
 * @return true if the caller should run one AUTO cycle now, false otherwise.
 */
bool ctrlAutoDue(Controller &controller, uint32_t nowMs);

/**
 * @brief Report how an AUTO cycle went
 *
 * @param[in,out] controller  State.
 * @param[in]     ok          true = clock fine and the right Nano answered,
 *                            false = anything failed (the LED blinks fast).
 */
void ctrlAutoResult(Controller &controller, bool ok);

/**
 * @brief Report a good MATLAB line that was sent to the right Nano
 *
 * Restarts the MATLAB timeout. Call it only for lines that parseAngles
 * accepted; broken lines are not reported, so the timeout keeps running.
 *
 * @param[in,out] controller  State.
 * @param[in]     nowMs       millis() now.
 * @param[in]     sendOk      true = the right Nano answered, false = it did
 *                            not (the LED blinks fast).
 */
void ctrlMatlabLine(Controller &controller, uint32_t nowMs, bool sendOk);

/**
 * @brief Check if MATLAB has just timed out
 *
 * In MATLAB mode, no good line for MATLAB_TIMEOUT_MS is a timeout. It sets
 * the error (fast blink) and is reported only once, until a good line or a
 * mode change restarts it.
 *
 * @param[in,out] controller  State.
 * @param[in]     nowMs       millis() now.
 *
 * @return true exactly once when the timeout hits, false otherwise.
 */
bool ctrlMatlabTimedOut(Controller &controller, uint32_t nowMs);

/**
 * @brief Say if the status LED should be on right now
 *
 * In priority order: error = fast blink (200 ms on, 200 ms off),
 * MATLAB = always on, AUTO = slow blink (1 s on, 1 s off).
 *
 * @param[in] controller  State.
 * @param[in] nowMs       millis() now.
 *
 * @return true = LED on, false = LED off.
 *
 * @note When millis() wraps after about 49.7 days, one blink phase is
 *       shorter. It only affects the LED.
 */
bool ctrlLedOn(const Controller &controller, uint32_t nowMs);

#endif /* CONTROLLER_H */