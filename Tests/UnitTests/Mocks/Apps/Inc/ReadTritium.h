/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#pragma once
#include_next "ReadTritium.h"
#ifndef TEST_READTRITIUM

#include "fff.h"

DECLARE_FAKE_VOID_FUNC(Task_ReadTritium, void*);
DECLARE_FAKE_VALUE_FUNC(float, Motor_RPM_Get);
DECLARE_FAKE_VALUE_FUNC(float, Motor_Velocity_Get);

#endif
