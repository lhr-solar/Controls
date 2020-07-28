#ifndef __BSP_SWITCHES_H
#define __BSP_SWITCHES_H

#include "common.h"
#include "config.h"
#include <bsp.h>

void BSP_Contactors_Init(void);
void read_Contactor_States(int16_t *contactorStates);
void BSP_Contactor_Set(int16_t motorContactorState, int16_t arrayContactorState);

#endif