/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#pragma once
#include_next "Minions.h"
#ifndef TEST_MINIONS

#include "fff.h"

DECLARE_FAKE_VOID_FUNC(Minions_Init);
DECLARE_FAKE_VALUE_FUNC(bool, Minions_Read, Pin);

#endif