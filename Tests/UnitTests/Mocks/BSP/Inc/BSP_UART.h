/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#pragma once
#include_next "BSP_UART.h"
#include "fff.h"

DECLARE_FAKE_VOID_FUNC(BspUartInit, Uart);
DECLARE_FAKE_VALUE_FUNC(uint32_t, BspUartRead, Uart, char*);
DECLARE_FAKE_VALUE_FUNC(uint32_t, BspUartWrite, Uart, char*, uint32_t);