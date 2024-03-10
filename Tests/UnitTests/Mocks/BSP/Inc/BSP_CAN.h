/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#pragma once
#include_next "BSP_CAN.h"
#include "fff.h"

DECLARE_FAKE_VOID_FUNC(BspCanInit, Can, Callback, Callback, uint16_t*, uint8_t);
//DECLARE_FAKE_VALUE_FUNC(ErrorStatus, BspCanWrite, Can, uint32_t, uint8_t, uint8_t);
DECLARE_FAKE_VALUE_FUNC(ErrorStatus, BspCanRead, Can, uint32_t*, uint8_t*);