/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#include "BSP_SPI.h"

DEFINE_FAKE_VOID_FUNC(BSP_SPI_Init);
DEFINE_FAKE_VOID_FUNC(BSP_SPI_Write, uint8_t*, uint8_t);
DEFINE_FAKE_VOID_FUNC(BSP_SPI_Read, uint8_t*, uint8_t);