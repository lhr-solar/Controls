/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#include "UpdateDisplay.h"

DEFINE_FAKE_VALUE_FUNC(UpdateDisplayError, UpdateDisplayInit);
DEFINE_FAKE_VALUE_FUNC(UpdateDisplayError, UpdateDisplaySetPage, Page);
DEFINE_FAKE_VALUE_FUNC(UpdateDisplayError, UpdateDisplaySetSOC, uint8_t);
DEFINE_FAKE_VALUE_FUNC(UpdateDisplayError, UpdateDisplaySetSBPV, uint32_t);
DEFINE_FAKE_VALUE_FUNC(UpdateDisplayError, UpdateDisplaySetVelocity, uint32_t);
DEFINE_FAKE_VALUE_FUNC(UpdateDisplayError, UpdateDisplaySetAccel, uint8_t);
DEFINE_FAKE_VALUE_FUNC(UpdateDisplayError, UpdateDisplaySetArray, bool);
DEFINE_FAKE_VALUE_FUNC(UpdateDisplayError, UpdateDisplaySetMotor, bool);
DEFINE_FAKE_VALUE_FUNC(UpdateDisplayError, UpdateDisplaySetGear, TriState);
DEFINE_FAKE_VALUE_FUNC(UpdateDisplayError, UpdateDisplaySetRegenState, TriState);
DEFINE_FAKE_VALUE_FUNC(UpdateDisplayError, UpdateDisplaySetCruiseState, TriState);
DEFINE_FAKE_VOID_FUNC(UpdateDisplayClearQueue);