/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file ReadCarCAN.h
 * @brief
 *
 * @defgroup ReadCarCAN
 * @addtogroup ReadCarCAN
 * @{
 */

#ifndef READ_CAR_CAN_H
#define READ_CAR_CAN_H

#include "common.h"

/**
 * Task Prototype
*/
void TaskReadCarCan(void* p_arg);

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

/* @} */
