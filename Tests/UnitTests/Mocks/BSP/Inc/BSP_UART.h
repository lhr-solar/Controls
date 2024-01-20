/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#ifndef __BSP_UART_H
#define __BSP_UART_H

#include "fff.h"
#include "common.h"
//#include <bsp.h>

typedef enum {UART_2, UART_3, NUM_UART} UART_t;

DECLARE_FAKE_VOID_FUNC(BSP_UART_Init, UART_t);

DECLARE_FAKE_VALUE_FUNC(uint32_t, BSP_UART_Read, UART_t, char*);

DECLARE_FAKE_VALUE_FUNC(uint32_t, BSP_UART_Write, UART_t, char*, uint32_t);

#endif


