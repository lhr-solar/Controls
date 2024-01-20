/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#include "ReadTritium.h"

DEFINE_FAKE_VOID_FUNC(Task_ReadTritium, void*);
DEFINE_FAKE_VALUE_FUNC(float, Motor_RPM_Get);
DEFINE_FAKE_VALUE_FUNC(float, Motor_Velocity_Get);