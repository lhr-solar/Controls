/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#include "Contactors.h"

DEFINE_FAKE_VOID_FUNC(ContactorsInit);
DEFINE_FAKE_VALUE_FUNC(bool, ContactorsGet, Contactor);
DEFINE_FAKE_VALUE_FUNC(ErrorStatus, ContactorsSet, Contactor, bool, bool);