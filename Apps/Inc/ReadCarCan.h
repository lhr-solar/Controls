/**
 * @file ReadCarCan.h
 * @brief Reads the car’s CAN bus for values relevant to Controls, and handles
 * ignition sequence logic.
 *
 * Call ChargeEnableGet() to get whether regen braking / charging is enabled or
 * not. Starting the task will spawn the watchdog, handle the ignition sequence
 * logic in communication with BPS, and update the display with the State of
 * Charge and the Supplemental Voltage of the car.
 *
 */

#ifndef READ_CAR_CAN_H
#define READ_CAR_CAN_H

#include "common.h"

/**
 * Length of the array and motor precharge-bypass contactor (PBC) saturation
 * buffers
 */
#define SAT_BUF_LENGTH 5

/**
 * The Array Saturation Threshold is used to determine if Controls has
 * received a sufficient number of BPS's HV Array Enable Messages.
 * BPS Array Saturation threshold is halfway between 0 and max saturation value.
 */
#define ARRAY_SATURATION_THRESHOLD (((SAT_BUF_LENGTH + 1) * SAT_BUF_LENGTH) / 4)

/**
 * The Plus-Minus Saturation Threshold is used to determine if Controls has
 * received a sufficient number of BPS's HV Plus-Minus Enable Messages.
 * BPS Plus-Minus Saturation threshold is halfway between 0 and max saturation
 * value.
 */
#define PLUS_MINUS_SATURATION_THRESHOLD \
    (((SAT_BUF_LENGTH + 1) * SAT_BUF_LENGTH) / 4)

/**
 * Delay time in milliseconds for the CAN Watchdog Timer
 */
#define CAN_WATCH_TMR_DLY_MS 500u

/**
 * Delay time in milliseconds for the Motor Precharge.
 */
#define PRECHARGE_MOTOR_DELAY 100u

/**
 * Delay time in milliseconds for the Array Precharge.
 */
#define PRECHARGE_ARRAY_DELAY 100u

/**
 * Error types
 */
typedef enum {
    kReadCarCanErrNone,
    kReadCarCanErrChargeDisable,  // Received a charge disable msg
    kReadCarCanErrMissedMsg,      // Didn't receive a BPS charge msg in time
    kReadCarCanErrDisableContactorsMsg,  // Ignition is turned to neither (off
                                         // due to LV) or both at the same time
                                         // (impossible) are on at
    kReadCarCanErrBpsTrip                // Received a BPS trip msg (0 or 1)
} ReadCarCanErrorCode;

/**
 * @brief Returns whether regen braking / charging is enabled or not
 * @return  Whether regen braking / charging is enabled or not
 */
bool ChargeEnableGet(void);

#endif
