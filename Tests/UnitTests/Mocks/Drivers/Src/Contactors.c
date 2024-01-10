#include "fff.h"
#include "Contactors.h"

DEFINE_FAKE_VOID_FUNC(Contactors_Init);
DEFINE_FAKE_VALUE_FUNC(bool, Contactors_Get, contactor_t);
DEFINE_FAKE_VALUE_FUNC(ErrorStatus, Contactors_Set, contactor_t, bool, bool);