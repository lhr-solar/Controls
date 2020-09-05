#ifndef __BSP_Lights_H
#define __BSP_ADC_H

#include "bsp.h"
#include "common.h"
#include "config.h"

typedef enum {M_CNCTR=0,RSVD_LED,CTRL_FAULT,RIGHT_BLINK,HeadlightPWR,A_CNCTR,BPS_FAULT,LEFT_BLINK,BPS_PWR,BrakeLight} LIGHT_t;
//external: turn signals (left,right); brake lights; internal: 
void BSP_Lights_Init();
state_t BSP_Lights_Read(LIGHT_t LightChannel);



#endif