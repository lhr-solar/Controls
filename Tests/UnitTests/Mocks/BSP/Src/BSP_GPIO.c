#include "BSP_GPIO.h"
#include "fff.h"


DEFINE_FAKE_VOID_FUNC(BSP_GPIO_Write_Pin, port_t, uint16_t, bool);
DEFINE_FAKE_VOID_FUNC(BSP_GPIO_Init, port_t, uint16_t, direction_t);
//FAKE_VOID_FUNC3(OSMutexCreate, struct os_mutex, CPU_CHAR, OS_ERR);
DEFINE_FAKE_VALUE_FUNC(uint8_t,BSP_GPIO_Get_State, port_t, uint16_t );
//FAKE_VOID_FUNC5(OSMutexPend, OS_MUTEX, OS_TICK, OS_OPT, CPU_TS, OS_ERR);
//FAKE_VOID_FUNC3(OSMutexPost, OS_MUTEX, OS_OPT, OS_ERR);
DEFINE_FAKE_VOID_FUNC(assertOSError, int);
