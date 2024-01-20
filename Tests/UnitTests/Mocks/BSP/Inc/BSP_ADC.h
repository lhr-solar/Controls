/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#pragma once
#include_next "BSP_ADC.h"
#include "fff.h"

DECLARE_FAKE_VOID_FUNC(BSP_ADC_Init);
DECLARE_FAKE_VALUE_FUNC(int16_t, BSP_ADC_Get_Millivoltage, ADC_t);
DECLARE_FAKE_VALUE_FUNC(int16_t, BSP_ADC_Get_Value, ADC_t);