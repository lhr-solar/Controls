/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#pragma once
#include_next "ReadCarCAN.h"
#ifndef TEST_READCARCAN

#include "fff.h"

DECLARE_FAKE_VOID_FUNC(Task_ReadCarCAN, void*);
DECLARE_FAKE_VALUE_FUNC(bool, ChargeEnable_Get);

#endif

