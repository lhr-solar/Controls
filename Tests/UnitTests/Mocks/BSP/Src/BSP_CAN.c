/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#include "BSP_CAN.h"

DEFINE_FAKE_VOID_FUNC(BSP_CAN_Init, CAN_t, callback_t, callback_t, uint16_t*, uint8_t);
DEFINE_FAKE_VALUE_FUNC(ErrorStatus, BSP_CAN_Write, CAN_t, uint32_t, uint8_t*, uint8_t);
DEFINE_FAKE_VALUE_FUNC(ErrorStatus, BSP_CAN_Read, CAN_t, uint32_t*, uint8_t*);