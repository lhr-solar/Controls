/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#pragma once
#include_next "Contactors.h"
#ifndef TEST_CONTACTORS
#include "fff.h"

DECLARE_FAKE_VOID_FUNC(ContactorsInit);
DECLARE_FAKE_VALUE_FUNC(bool, ContactorsGet, Contactor);
DECLARE_FAKE_VALUE_FUNC(ErrorStatus, ContactorsSet, Contactor, bool, bool);

#endif

