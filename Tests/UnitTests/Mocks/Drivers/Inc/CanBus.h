/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#pragma once
#include_next "CanBus.h"
#ifndef TEST_CANBUS

#include "fff.h"

DECLARE_FAKE_VALUE_FUNC(ErrorStatus, CanBusInit, Can, CanId*, uint8_t);
DECLARE_FAKE_VALUE_FUNC(ErrorStatus, CanBusSend, CanData, bool, Can);
DECLARE_FAKE_VALUE_FUNC(ErrorStatus, CanBusRead, CanData*, bool, Can);

#endif