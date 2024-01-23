/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#pragma once
#include_next "BSP_GPIO.h"
#include "fff.h"

DECLARE_FAKE_VOID_FUNC(BspGpioInit, Port, uint16_t, Direction);
DECLARE_FAKE_VALUE_FUNC(uint16_t, BspGpioRead, Port);
DECLARE_FAKE_VOID_FUNC(BspGpioWrite, Port, uint16_t);
DECLARE_FAKE_VALUE_FUNC(uint8_t, BspGpioReadPin, Port, uint16_t);
DECLARE_FAKE_VOID_FUNC(BspGpioWritePin, Port, uint16_t, bool);
DECLARE_FAKE_VALUE_FUNC(uint8_t, BspGpioGetState, Port, uint16_t);