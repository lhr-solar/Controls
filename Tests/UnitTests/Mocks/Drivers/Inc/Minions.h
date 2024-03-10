/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#pragma once
#include_next "Minions.h"
#ifndef TEST_MINIONS

#include "fff.h"

DECLARE_FAKE_VOID_FUNC(MinionsInit);
DECLARE_FAKE_VALUE_FUNC(bool, MinionsRead, Pin);

#endif