#include "fff.h"
#include "Minions.h"

DEFINE_FAKE_VOID_FUNC(Minions_Init);

DEFINE_FAKE_VALUE_FUNC(bool, Minions_Read, pin_t);
