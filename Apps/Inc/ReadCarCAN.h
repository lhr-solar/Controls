/**
 * @file
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @brief
 *
 * @defgroup ReadCarCAN
 * @addtogroup ReadCarCAN
 * @{
 */

#ifndef __READ_CAR_CAN_H
#define __READ_CAR_CAN_H

#include "CANbus.h"
#include "Tasks.h"
#include "common.h"
#include "os.h"

#define SAT_BUF_LENGTH 5

/**
 * @brief Returns whether regen braking / charging is enabled or not
 * @return  Whether regen braking / charging is enabled or not
 */
bool ChargeEnable_Get();

#endif

/* @} */
