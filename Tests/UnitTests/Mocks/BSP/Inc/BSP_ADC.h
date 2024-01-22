/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#pragma once
#include_next "BSP_ADC.h"
#include "fff.h"

DECLARE_FAKE_VOID_FUNC(BspAdcInit);
DECLARE_FAKE_VALUE_FUNC(int16_t, BspAdcGetValue, Adc);
DECLARE_FAKE_VALUE_FUNC(int16_t, BspAdcGetMillivoltage, Adc);