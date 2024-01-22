/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#pragma once
#include_next "UpdateDisplay.h"
#ifndef TEST_UPDATEDISPLAY

#include "fff.h"

DECLARE_FAKE_VALUE_FUNC(UpdateDisplayError, UpdateDisplayInit);
DECLARE_FAKE_VALUE_FUNC(UpdateDisplayError, UpdateDisplaySetPage, Page);
DECLARE_FAKE_VALUE_FUNC(UpdateDisplayError, UpdateDisplaySetSOC, uint8_t);
DECLARE_FAKE_VALUE_FUNC(UpdateDisplayError, UpdateDisplaySetSBPV, uint32_t);
DECLARE_FAKE_VALUE_FUNC(UpdateDisplayError, UpdateDisplaySetVelocity, uint32_t);
DECLARE_FAKE_VALUE_FUNC(UpdateDisplayError, UpdateDisplaySetAccel, uint8_t);
DECLARE_FAKE_VALUE_FUNC(UpdateDisplayError, UpdateDisplaySetArray, bool);
DECLARE_FAKE_VALUE_FUNC(UpdateDisplayError, UpdateDisplaySetMotor, bool);
DECLARE_FAKE_VALUE_FUNC(UpdateDisplayError, UpdateDisplaySetGear, TriState);
DECLARE_FAKE_VALUE_FUNC(UpdateDisplayError, UpdateDisplaySetRegenState, TriState);
DECLARE_FAKE_VALUE_FUNC(UpdateDisplayError, UpdateDisplaySetCruiseState, TriState);
DECLARE_FAKE_VOID_FUNC(UpdateDisplayClearQueue);

#endif