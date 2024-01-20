/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#pragma once
#include_next "BSP_GPIO.h"
#include "fff.h"

DECLARE_FAKE_VOID_FUNC(BSP_GPIO_Init, port_t, uint16_t, direction_t);
DECLARE_FAKE_VALUE_FUNC(uint16_t, BSP_GPIO_Read, port_t);
DECLARE_FAKE_VOID_FUNC(BSP_GPIO_Write, port_t, uint16_t);
DECLARE_FAKE_VALUE_FUNC(uint8_t, BSP_GPIO_Read_Pin, port_t, uint16_t);
DECLARE_FAKE_VOID_FUNC(BSP_GPIO_Write_Pin, port_t, uint16_t, bool);
DECLARE_FAKE_VALUE_FUNC(uint8_t, BSP_GPIO_Get_State, port_t, uint16_t);