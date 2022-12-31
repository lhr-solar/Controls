/* Copyright (c) 2020 UT Longhorn Racing Solar */

#ifndef __READ_CAR_CAN_H
#define __READ_CAR_CAN_H

#include "os.h"
#include "common.h"
#include "Tasks.h"

#include "CANbus.h"
extern bool ChargeEnable;


#define SAT_BUF_LENGTH 5

/**
 * @brief Returns whether regen braking is enabled or not
 * @return  Whether regen braking is enabled or not
*/
bool RegenEnable_Get();

#endif