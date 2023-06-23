/* Copyright (c) 2020 UT Longhorn Racing Solar */

#ifndef __READ_CAR_CAN_H
#define __READ_CAR_CAN_H

#include "os.h"
#include "common.h"
#include "Tasks.h"

#include "CANbus.h"

#define SAT_BUF_LENGTH 5

#ifdef __TEST_READCARCAN
extern int chargeMsgSaturation;
extern uint8_t SOC;
extern uint32_t SBPV; 
#endif

/**
 * @brief Returns whether regen braking / charging is enabled or not
 * @return  Whether regen braking / charging is enabled or not
*/
bool ChargeEnable_Get();

#endif