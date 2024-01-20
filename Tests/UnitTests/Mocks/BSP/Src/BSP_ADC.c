/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#include "BSP_ADC.h"

DEFINE_FAKE_VOID_FUNC(BSP_ADC_Init);
DEFINE_FAKE_VALUE_FUNC(int16_t, BSP_ADC_Get_Millivoltage, ADC_t);
DEFINE_FAKE_VALUE_FUNC(int16_t, BSP_ADC_Get_Value, ADC_t);