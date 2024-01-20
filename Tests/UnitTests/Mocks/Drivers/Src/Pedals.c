/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#include "Pedals.h"

DEFINE_FAKE_VOID_FUNC(Pedals_Init);

DEFINE_FAKE_VALUE_FUNC(int8_t, Pedals_Read, pedal_t);