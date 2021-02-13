/* Copyright (c) 2020 UT Longhorn Racing Solar */


#ifndef __RTOS_PEDALS_H
#define __RTOS_PEDALS_H

#include "common.h"
#include "config.h"
#include "Pedals.h"
#include "Tasks.h"
#include "os.h"

// Semaphores
extern OS_SEM VelocityChange_Sem4;
extern OS_SEM LightsChange_Sem4;

/**
* @brief   Read both pedal percentage values
* @param   None
* @return  None
*/ 
void Task_ReadPedals(void *p_arg);

/**
* @brief   Getter function for local variable brake_percentage
* @param   None
* @return  brake_percentage
*/ 
uint8_t RTOSPedals_GetBrakePercentage(void);

/**
* @brief   Getter function for local variable accelerator_percentage
* @param   None
* @return  accelerator_percentage
*/ 
uint8_t RTOSPedals_GetAcceleratorPercentage(void);

#endif