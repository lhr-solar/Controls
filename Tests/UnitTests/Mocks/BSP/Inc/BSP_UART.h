/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#pragma once
#include_next "BSP_UART.h"
#include "fff.h"

DECLARE_FAKE_VOID_FUNC(BSP_UART_Init, UART_t);
DECLARE_FAKE_VALUE_FUNC(uint32_t, BSP_UART_Read, UART_t, char*);
DECLARE_FAKE_VALUE_FUNC(uint32_t, BSP_UART_Write, UART_t, char*, uint32_t);