/* Copyright (c) 2020 UT Longhorn Racing Solar */

#ifndef __BLINKER_LIGHT_H
#define __BLINKER_LIGHT_H

#include "common.h"
#include "Tasks.h"
#include "os.h"

#include "Lights.h"

extern car_state_t car_state;

/**
* @brief   Toggle proper indicator lights
* @param   None
* @return  None
*/ 
void Task_BlinkLight(void *p_arg);

#endif

