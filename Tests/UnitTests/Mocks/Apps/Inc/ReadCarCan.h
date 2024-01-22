/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#pragma once
#include_next "ReadCarCan.h"
#ifndef TEST_READCARCAN

#include "fff.h"

DECLARE_FAKE_VOID_FUNC(TaskReadCarCan, void*);
DECLARE_FAKE_VALUE_FUNC(bool, ChargeEnableGet);

#endif

