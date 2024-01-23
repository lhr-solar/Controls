/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#include "ReadCarCan.h"
#include "fff.h"


DEFINE_FAKE_VOID_FUNC(TaskReadCarCan, void*);
DEFINE_FAKE_VALUE_FUNC(bool, ChargeEnableGet);