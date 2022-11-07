/* Copyright (c) 2020 UT Longhorn Racing Solar */

#ifndef __READ_CAR_CAN_H
#define __READ_CAR_CAN_H

#include "os.h"
#include "common.h"
#include "Tasks.h"

#include "CANbus.h"

#define SAT_BUF_LENGTH 5

/**
 * @brief Returns whether regen braking is on or off
 * @return  Whether regen braking is on or off
*/
bool RegenEnable_Get();

#endif