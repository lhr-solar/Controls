#include "unity.h"
#include "fff.h"
#include "Contactors.h"
#include "BSP_GPIO.h"

DEFINE_FFF_GLOBALS;

void setUp(void) {}

void tearDown(void) {}

FAKE_VOID_FUNC3(BSP_GPIO_Write_Pin, port_t, uint16_t, bool);
FAKE_VOID_FUNC3(BSP_GPIO_Init, port_t, uint16_t, direction_t);
//FAKE_VOID_FUNC3(OSMutexCreate, struct os_mutex, CPU_CHAR, OS_ERR);
FAKE_VALUE_FUNC2(uint8_t,BSP_GPIO_Get_State, port_t, uint16_t );
//FAKE_VOID_FUNC5(OSMutexPend, OS_MUTEX, OS_TICK, OS_OPT, CPU_TS, OS_ERR);
//FAKE_VOID_FUNC3(OSMutexPost, OS_MUTEX, OS_OPT, OS_ERR);
FAKE_VOID_FUNC1(assertOSError, OS_ERR);


void test_UnitTestContactors(void){
    setContactor(ARRAY_PRECHARGE_BYPASS_CONTACTOR, ON);
}


