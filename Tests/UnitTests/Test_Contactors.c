#define MOCK_BSP_GPIO
#include "Unity/unity.h"
#include "../../Drivers/Inc/Contactors.h"


void setUp(void) {}

void tearDown(void) {}

void test_UnitTestContactors(void){
    Contactors_Set(ARRAY_PRECHARGE_BYPASS_CONTACTOR, true, true);
}

/*=======MAIN=====*/
int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_UnitTestContactors);
  return UNITY_END();
}