/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#include "Contactors.h"

DEFINE_FAKE_VOID_FUNC(Contactors_Init);
DEFINE_FAKE_VALUE_FUNC(bool, Contactors_Get, Contactor);
DEFINE_FAKE_VALUE_FUNC(ErrorStatus, Contactors_Set, Contactor, bool, bool);