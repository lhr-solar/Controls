/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#ifndef __BSP_CAN_H
#define __BSP_CAN_H

#include "fff.h"
#include "common.h"
#include "config.h"
//#include <bsp.h>

typedef enum {CAN_1=0, CAN_3, NUM_CAN} CAN_t;

DECLARE_FAKE_VOID_FUNC(BSP_CAN_Init, CAN_t, callback_t, callback_t, uint16_t*, uint8_t);

DECLARE_FAKE_VALUE_FUNC(ErrorStatus, BSP_CAN_Write, CAN_t, uint32_t, uint8_t*, uint8_t);

DECLARE_FAKE_VALUE_FUNC(ErrorStatus, BSP_CAN_Read, CAN_t, uint32_t*, uint8_t*);

#endif
