/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#pragma once
#include_next "Pedals.h"
#ifndef TEST_PEDALS

#include "fff.h"

DECLARE_FAKE_VOID_FUNC(Pedals_Init);
DECLARE_FAKE_VALUE_FUNC(int8_t, Pedals_Read, pedal_t);

#endif
