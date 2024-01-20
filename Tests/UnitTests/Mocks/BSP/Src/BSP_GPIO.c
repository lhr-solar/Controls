/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#include "BSP_GPIO.h"
#include "fff.h"


DEFINE_FAKE_VOID_FUNC(BSP_GPIO_Init, port_t, uint16_t, direction_t);

DEFINE_FAKE_VALUE_FUNC(uint16_t, BSP_GPIO_Read, port_t);

DEFINE_FAKE_VOID_FUNC(BSP_GPIO_Write, port_t, uint16_t);

DEFINE_FAKE_VALUE_FUNC(uint8_t, BSP_GPIO_Read_Pin, port_t, uint16_t);

DEFINE_FAKE_VOID_FUNC(BSP_GPIO_Write_Pin, port_t, uint16_t, bool);

DEFINE_FAKE_VALUE_FUNC(uint8_t, BSP_GPIO_Get_State, port_t, uint16_t);

