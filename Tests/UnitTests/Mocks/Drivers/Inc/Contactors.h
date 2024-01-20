/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#pragma once
#include_next "Contactors.h"
#ifndef TEST_CONTACTORS
#include "fff.h"

DECLARE_FAKE_VOID_FUNC(Contactors_Init);
DECLARE_FAKE_VALUE_FUNC(bool, Contactors_Get, contactor_t);
DECLARE_FAKE_VALUE_FUNC(ErrorStatus, Contactors_Set, contactor_t, bool, bool);

#endif

