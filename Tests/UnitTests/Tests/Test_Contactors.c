//////////////////////////////////////////
//////////////////////////////////////////
// Unity required for this testing format
#include "unity.h"
//////////////////////////////////////////
//////////////////////////////////////////
// The header file we are unit testing:
#include "Contactors.h"
//////////////////////////////////////////
// fff header so we can DEFINE_FFF_GLOBALS
// (can only happen once per test)
#include "fff.h"

DEFINE_FFF_GLOBALS;

void setUp(void) {}

void tearDown(void) {}

void test_UnitTestContactors_Init(void){
    ContactorsInit();
}

void test_UnitTestContactors_Get(void){
   // Contactors_Get(1);
    BspGpioGetState_fake.return_val = 1;
    printf("VALUE:%d\n", ContactorsGet(1));

    BspGpioGetState_fake.return_val = 0;
    printf("VALUE:%d\n", ContactorsGet(1));
    for(int x = 500; x < 1000; x+=100){
       
    }
}

void test_UnitTestContactors_Set(void){
   printf("VALUE:%d\n", ContactorsSet(1, 1, 1));

   BspGpioGetState_fake.return_val = 1;
   printf("VALUE:%d\n", ContactorsSet(1, 1, 1));
   

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