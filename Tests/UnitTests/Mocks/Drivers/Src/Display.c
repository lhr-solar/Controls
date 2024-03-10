/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#include "Display.h"

DEFINE_FAKE_VALUE_FUNC(DisplayError, DisplaySend, DisplayCmd);
DEFINE_FAKE_VALUE_FUNC(DisplayError, DisplayInit);
DEFINE_FAKE_VALUE_FUNC(DisplayError, DisplayReset);
DEFINE_FAKE_VALUE_FUNC(DisplayError, DisplayFault, ErrorCode);
DEFINE_FAKE_VALUE_FUNC(DisplayError, DisplayEvac, uint8_t, uint32_t);