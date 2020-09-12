#ifndef __BSP_Lights_H
#define __BSP_Lights_H

#include <bsp.h>
#include "common.h"
#include "config.h"

typedef enum {M_CNCTR=0,RSVD_LED,CTRL_FAULT,RIGHT_BLINK,HeadlightPWR,A_CNCTR,BPS_FAULT,LEFT_BLINK,BPS_PWR,BrakeLight} LIGHT_t;
typedef enum {ON=0, OFF} LIGHT_state;

void BSP_Lights_Init();
state_t BSP_Lights_Read(LIGHT_t LightChannel, LIGHT_state lightState);



#endif