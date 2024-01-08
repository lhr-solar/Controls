//////////////////////////////////////////
//////////////////////////////////////////
// Unity required for this testing format
#include "unity.h" 
//////////////////////////////////////////
//////////////////////////////////////////
// The header file we are unit testing:
#include "Pedals.h" 
//////////////////////////////////////////
// fff header so we can DEFINE_FFF_GLOBALS
// (can only happen once per test)
#include "fff.h"

DEFINE_FFF_GLOBALS;

void setUp(void) {}

void tearDown(void) {}

void test_UnitTestPedals_Init(void){
    Pedals_Init(); 
}

void test_UnitTestPedals_Read(void){
    for(int x = 500; x < 1000; x+=100){
        BSP_ADC_Get_Millivoltage_fake.return_val = x; 
        Pedals_Read(ACCELERATOR); 
        printf("\n\r%d\r", Pedals_Read(ACCELERATOR));
    }
}

void test_UnitTestPedals_Read_This_Will_Fail(void){
   TEST_ASSERT_EQUAL(Pedals_Read(ACCELERATOR), 30); 
}



/*=======MAIN=====*/
int main(void)
{
  UNITY_BEGIN(); 
  RUN_TEST(test_UnitTestPedals_Init); 
  RUN_TEST(test_UnitTestPedals_Read);
  RUN_TEST(test_UnitTestPedals_Read_This_Will_Fail); 
  return UNITY_END(); 
}