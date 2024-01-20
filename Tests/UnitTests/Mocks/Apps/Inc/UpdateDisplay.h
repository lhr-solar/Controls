/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#pragma once
#include_next "UpdateDisplay.h"
#ifndef TEST_UPDATEDISPLAY

#include "fff.h"

DECLARE_FAKE_VALUE_FUNC(UpdateDisplayError_t, UpdateDisplay_Init);
DECLARE_FAKE_VALUE_FUNC(UpdateDisplayError_t, UpdateDisplay_SetPage, Page_t);
DECLARE_FAKE_VALUE_FUNC(UpdateDisplayError_t, UpdateDisplay_SetSOC, uint8_t);
DECLARE_FAKE_VALUE_FUNC(UpdateDisplayError_t, UpdateDisplay_SetSBPV, uint32_t);
DECLARE_FAKE_VALUE_FUNC(UpdateDisplayError_t, UpdateDisplay_SetVelocity, uint32_t);
DECLARE_FAKE_VALUE_FUNC(UpdateDisplayError_t, UpdateDisplay_SetAccel, uint8_t);
DECLARE_FAKE_VALUE_FUNC(UpdateDisplayError_t, UpdateDisplay_SetArray, bool);
DECLARE_FAKE_VALUE_FUNC(UpdateDisplayError_t, UpdateDisplay_SetMotor, bool);
DECLARE_FAKE_VALUE_FUNC(UpdateDisplayError_t, UpdateDisplay_SetGear, TriState_t);
DECLARE_FAKE_VALUE_FUNC(UpdateDisplayError_t, UpdateDisplay_SetRegenState, TriState_t);
DECLARE_FAKE_VALUE_FUNC(UpdateDisplayError_t, UpdateDisplay_SetCruiseState, TriState_t);
DECLARE_FAKE_VOID_FUNC(UpdateDisplay_ClearQueue);

#endif