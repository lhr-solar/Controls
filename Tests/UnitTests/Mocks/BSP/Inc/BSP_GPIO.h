
#ifndef __BSP_GPIO_H
#define __BSP_GPIO_H

#include "common.h"
#include "fff.h"


typedef enum {PORTA = 0, PORTB, PORTC, PORTD, NUM_PORTS} port_t; 
typedef enum {INPUT = 0, OUTPUT} direction_t;

DECLARE_FAKE_VOID_FUNC(BSP_GPIO_Write_Pin, port_t, uint16_t, bool);
DECLARE_FAKE_VOID_FUNC(BSP_GPIO_Init, port_t, uint16_t, direction_t);
//FAKE_VOID_FUNC3(OSMutexCreate, struct os_mutex, CPU_CHAR, OS_ERR);
DECLARE_FAKE_VALUE_FUNC(uint8_t,BSP_GPIO_Get_State, port_t, uint16_t );
//FAKE_VOID_FUNC5(OSMutexPend, OS_MUTEX, OS_TICK, OS_OPT, CPU_TS, OS_ERR);
//FAKE_VOID_FUNC3(OSMutexPost, OS_MUTEX, OS_OPT, OS_ERR);
DECLARE_FAKE_VOID_FUNC(assertOSError, int);


#endif


/* @} */
