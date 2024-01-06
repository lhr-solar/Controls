#ifndef __BSP_ADC_H
#define __BSP_ADC_H

#include "common.h"
#include "fff.h"



DECLARE_FAKE_VOID_FUNC(BSP_ADC_Init);
DECLARE_FAKE_VALUE_FUNC(int16_t, BSP_ADC_Get_Millivoltage, int);
DECLARE_FAKE_VALUE_FUNC(int16_t, BSP_ADC_Get_Value, int);

#endif