/* Copyright (c) 2020 UT Longhorn Racing Solar */

#ifndef __READ_CAR_CAN_H
#define __READ_CAR_CAN_H

#include "Tasks.h"
#include "common.h"
#include "os.h"

#include "CANbus.h"

#define SAT_BUF_LENGTH 5

/**
 * @brief Returns whether regen braking / charging is enabled or not
 * @return  Whether regen braking / charging is enabled or not
 */
bool ChargeEnable_Get();

#endif