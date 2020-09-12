#ifndef __BSP_Lights_H
#define __BSP_Lights_H

#include <bsp.h>
#include "common.h"
#include "config.h"


typedef enum {M_CNCTR=0,RSVD_LED,CTRL_FAULT,RIGHT_BLINK,HeadlightPWR,A_CNCTR,BPS_FAULT,LEFT_BLINK,BPS_PWR,BrakeLight} LIGHT_t;

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
state_t BSP_Lights_Read(LIGHT_t LightChannel);

/**
* @brief   Switch light between ON/OFF states
* @param   LightChannel Which light to switch
* @return  void
*/ 
void BSP_Lights_Switch(LIGHT_t LightChannel);



#endif