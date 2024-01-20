/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#ifndef __BSP_SPI_H
#define __BSP_SPI_H

#include "fff.h"
#include "common.h"



DECLARE_FAKE_VOID_FUNC(BSP_SPI_Init);

DECLARE_FAKE_VOID_FUNC(BSP_SPI_Write, uint8_t*, uint8_t);

DECLARE_FAKE_VOID_FUNC(BSP_SPI_Read, uint8_t*, uint8_t);

#endif


