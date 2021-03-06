/* Copyright (c) 2020 UT Longhorn Racing Solar */


#ifndef __RTOS_PEDALS_H
#define __RTOS_PEDALS_H

#include "common.h"
#include "config.h"
#include "Pedals.h"
#include "Tasks.h"
#include "os.h"

extern car_state_t car_state;

/**
* @brief   Read both pedal percentage values
* @param   None
* @return  None
*/ 
void Task_ReadPedals(void *p_arg);


#endif