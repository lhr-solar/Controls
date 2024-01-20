/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#include "UpdateDisplay.h"

DEFINE_FAKE_VALUE_FUNC(UpdateDisplayError_t, UpdateDisplay_Init);
DEFINE_FAKE_VALUE_FUNC(UpdateDisplayError_t, UpdateDisplay_SetPage, Page_t);
DEFINE_FAKE_VALUE_FUNC(UpdateDisplayError_t, UpdateDisplay_SetSOC, uint8_t);
DEFINE_FAKE_VALUE_FUNC(UpdateDisplayError_t, UpdateDisplay_SetSBPV, uint32_t);
DEFINE_FAKE_VALUE_FUNC(UpdateDisplayError_t, UpdateDisplay_SetVelocity, uint32_t);
DEFINE_FAKE_VALUE_FUNC(UpdateDisplayError_t, UpdateDisplay_SetAccel, uint8_t);
DEFINE_FAKE_VALUE_FUNC(UpdateDisplayError_t, UpdateDisplay_SetArray, bool);
DEFINE_FAKE_VALUE_FUNC(UpdateDisplayError_t, UpdateDisplay_SetMotor, bool);
DEFINE_FAKE_VALUE_FUNC(UpdateDisplayError_t, UpdateDisplay_SetGear, TriState_t);
DEFINE_FAKE_VALUE_FUNC(UpdateDisplayError_t, UpdateDisplay_SetRegenState, TriState_t);
DEFINE_FAKE_VALUE_FUNC(UpdateDisplayError_t, UpdateDisplay_SetCruiseState, TriState_t);
DEFINE_FAKE_VOID_FUNC(UpdateDisplay_ClearQueue);