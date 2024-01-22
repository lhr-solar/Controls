/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#pragma once
#include_next "ReadTritium.h"
#ifndef TEST_READTRITIUM

#include "fff.h"

DECLARE_FAKE_VOID_FUNC(TaskReadTritium, void*);
DECLARE_FAKE_VALUE_FUNC(float, MotorRpmGet);
DECLARE_FAKE_VALUE_FUNC(float, MotorVelocityGet);

#endif
