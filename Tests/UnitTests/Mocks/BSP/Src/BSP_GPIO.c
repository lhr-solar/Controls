/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#include "BSP_GPIO.h"

DEFINE_FAKE_VOID_FUNC(BspGpioInit, Port, uint16_t, Direction);
DEFINE_FAKE_VALUE_FUNC(uint16_t, BspGpioRead, Port);
DEFINE_FAKE_VOID_FUNC(BspGpioWrite, Port, uint16_t);
DEFINE_FAKE_VALUE_FUNC(uint8_t, BspGpioReadPin, Port, uint16_t);
DEFINE_FAKE_VOID_FUNC(BspGpioWritePin, Port, uint16_t, bool);
DEFINE_FAKE_VALUE_FUNC(uint8_t, BspGpioGetState, Port, uint16_t);