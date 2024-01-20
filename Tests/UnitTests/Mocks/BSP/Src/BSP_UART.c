/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#include "BSP_UART.h"

DEFINE_FAKE_VOID_FUNC(BSP_UART_Init, UART_t);
DEFINE_FAKE_VALUE_FUNC(uint32_t, BSP_UART_Read, UART_t, char*);
DEFINE_FAKE_VALUE_FUNC(uint32_t, BSP_UART_Write, UART_t, char*, uint32_t);