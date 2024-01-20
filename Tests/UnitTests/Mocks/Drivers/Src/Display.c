/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#include "Display.h"

DEFINE_FAKE_VALUE_FUNC(DisplayError_t, Display_Send, DisplayCmd_t);
DEFINE_FAKE_VALUE_FUNC(DisplayError_t, Display_Init);
DEFINE_FAKE_VALUE_FUNC(DisplayError_t, Display_Reset);
DEFINE_FAKE_VALUE_FUNC(DisplayError_t, Display_Error, error_code_t);
DEFINE_FAKE_VALUE_FUNC(DisplayError_t, Display_Evac, uint8_t, uint32_t);