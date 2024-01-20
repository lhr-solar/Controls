/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#pragma once
#include_next "CANbus.h"
#ifndef TEST_CANBUS

#include "fff.h"

DECLARE_FAKE_VALUE_FUNC(ErrorStatus, CANbus_Init, CAN_t, CANId_t*, uint8_t);
DECLARE_FAKE_VALUE_FUNC(ErrorStatus, CANbus_Send, CANDATA_t, bool, CAN_t);
DECLARE_FAKE_VALUE_FUNC(ErrorStatus, CANbus_Read, CANDATA_t*, bool, CAN_t);

#endif