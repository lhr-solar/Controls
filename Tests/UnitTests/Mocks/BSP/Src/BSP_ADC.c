/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#include "BSP_ADC.h"

DEFINE_FAKE_VOID_FUNC(BspAdcInit);
DEFINE_FAKE_VALUE_FUNC(int16_t, BspAdcGetValue, Adc);
DEFINE_FAKE_VALUE_FUNC(int16_t, BspAdcGetMillivoltage, Adc);