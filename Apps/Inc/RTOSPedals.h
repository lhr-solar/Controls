/* Copyright (c) 2020 UT Longhorn Racing Solar */


#ifndef __RTOS_PEDALS_H
#define __RTOS_PEDALS_H

#include "common.h"
#include "config.h"
#include "Pedals.h"
#include "Tasks.h"
#include "os.h"

//Globals located in task.h
extern int8_t accelerator_percentage;
extern int8_t brake_percentage;

// Semaphores
extern OS_SEM VelocityChange_Sem4;
extern OS_SEM LightsChange_Sem4;

/**
* @brief   Read both pedal percentage values
* @param   None
* @return  None
*/ 
void Task_ReadPedals(void *p_arg);

#endif