/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#include "ReadCarCAN.h"

DEFINE_FAKE_VOID_FUNC(Task_ReadCarCAN, void*);
DEFINE_FAKE_VALUE_FUNC(bool, ChargeEnable_Get);