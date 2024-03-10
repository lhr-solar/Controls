/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#include "BSP_CAN.h"

DEFINE_FAKE_VOID_FUNC(BspCanInit, Can, Callback, Callback, uint16_t*, uint8_t);
//DEFINE_FAKE_VALUE_FUNC(ErrorStatus, BspCanWrite, Can, uint32_t, uint8_t, uint8_t);
DEFINE_FAKE_VALUE_FUNC(ErrorStatus, BspCanRead, Can, uint32_t*, uint8_t*);