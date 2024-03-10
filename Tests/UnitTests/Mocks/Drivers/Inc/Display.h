/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#pragma once
#include_next "Display.h"
#ifndef TEST_DISPLAY

#include "fff.h"

DECLARE_FAKE_VALUE_FUNC(DisplayError, DisplaySend, DisplayCmd);
DECLARE_FAKE_VALUE_FUNC(DisplayError, DisplayInit);
DECLARE_FAKE_VALUE_FUNC(DisplayError, DisplayReset);
DECLARE_FAKE_VALUE_FUNC(DisplayError, DisplayFault, ErrorCode);
DECLARE_FAKE_VALUE_FUNC(DisplayError, DisplayEvac, uint8_t, uint32_t);

#endif
