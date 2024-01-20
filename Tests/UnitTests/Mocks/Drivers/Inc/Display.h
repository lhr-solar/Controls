/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#pragma once
#include_next "Display.h"
#ifndef TEST_DISPLAY

#include "fff.h"

DECLARE_FAKE_VALUE_FUNC(DisplayError_t, Display_Send, DisplayCmd_t);
DECLARE_FAKE_VALUE_FUNC(DisplayError_t, Display_Init);
DECLARE_FAKE_VALUE_FUNC(DisplayError_t, Display_Reset);
DECLARE_FAKE_VALUE_FUNC(DisplayError_t, Display_Error, error_code_t);
DECLARE_FAKE_VALUE_FUNC(DisplayError_t, Display_Evac, uint8_t, uint32_t);

#endif
