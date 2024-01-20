/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#pragma once
#include_next "BSP_SPI.h"
#include "fff.h"

DECLARE_FAKE_VOID_FUNC(BSP_SPI_Init);
DECLARE_FAKE_VOID_FUNC(BSP_SPI_Write, uint8_t*, uint8_t);
DECLARE_FAKE_VOID_FUNC(BSP_SPI_Read, uint8_t*, uint8_t);