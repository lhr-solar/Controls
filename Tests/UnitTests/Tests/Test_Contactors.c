//////////////////////////////////////////
//////////////////////////////////////////
// Unity required for this testing format
#include "unity.h"
//////////////////////////////////////////
//////////////////////////////////////////
// The header file we are unit testing:
#include "Contactors.h"


void setUp(void) {}

void tearDown(void) {}

void test_UnitTestContactors_Init(void){
    Contactors_Init();
}

void test_UnitTestContactors_Get(void){
   // Contactors_Get(1);
    BSP_GPIO_Get_State_fake.return_val = 1;
    printf("VALUE:%d\n", Contactors_Get(1));

    BSP_GPIO_Get_State_fake.return_val = 0;
    printf("VALUE:%d\n", Contactors_Get(1));
    for(int x = 500; x < 1000; x+=100){
       
    }
}

void test_UnitTestContactors_Set(void){
   printf("VALUE:%d\n", Contactors_Set(ARRAY_PRECHARGE_BYPASS_CONTACTOR, 1, 1));

   BSP_GPIO_Get_State_fake.return_val = 1;
   printf("VALUE:%d\n", Contactors_Set(ARRAY_PRECHARGE_BYPASS_CONTACTOR, 1, 1));
   

}



/*=======MAIN=====*/
int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_UnitTestContactors_Init);
  RUN_TEST(test_UnitTestContactors_Get);
  RUN_TEST(test_UnitTestContactors_Set);
  return UNITY_END();
}