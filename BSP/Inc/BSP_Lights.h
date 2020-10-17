/* Copyright (c) 2020 UT Longhorn Racing Solar */

#ifndef __BSP_Lights_H
#define __BSP_Lights_H

#include <bsp.h>
#include "common.h"
#include "config.h"


typedef enum {M_CNCTR=0,RSVD_LED,CTRL_FAULT,RIGHT_BLINK,HeadlightPWR,A_CNCTR,BPS_FAULT,LEFT_BLINK,BPS_PWR,BrakeLight} light_t;

/**
* @brief   Initialize Lights Module
* @return  void
*/ 
void BSP_Lights_Init();

/**
* @brief   Read the state of the lights
* @param   LightChannel Which Light to read
* @return  returns state enum which indicates ON/OFF
*/ 
State BSP_Lights_Read(light_t LightChannel);

/**
 * @brief   Set light to given state
 * @param   LightChanel which light to set
 * @param   State what state to set the light to
 * @return  void
 */
void BSP_Lights_Set(light_t LightChannel, State State);



#endif
