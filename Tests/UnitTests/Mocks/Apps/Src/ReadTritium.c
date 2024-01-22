/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#include "ReadTritium.h"

DEFINE_FAKE_VOID_FUNC(TaskReadTritium, void*);
DEFINE_FAKE_VALUE_FUNC(float, MotorRpmGet);
DEFINE_FAKE_VALUE_FUNC(float, MotorVelocityGet);