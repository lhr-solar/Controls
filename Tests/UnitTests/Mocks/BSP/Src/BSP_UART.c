/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#include "BSP_UART.h"

DEFINE_FAKE_VOID_FUNC(BspUartInit, Uart);
DEFINE_FAKE_VALUE_FUNC(uint32_t, BspUartRead, Uart, char*);
DEFINE_FAKE_VALUE_FUNC(uint32_t, BspUartWrite, Uart, char*, uint32_t);