#ifndef __BSP_Lights_H
#define __BSP_ADC_H

#include "bsp.h"
#include "common.h"
#include "config.h"

typedef enum {LIGHT_0,LIGHT_1,NUMBER_OF_LIGHTS} LIGHT_t;

void BSP_Lights_Init(LIGHT_t LightChannel);



#endif