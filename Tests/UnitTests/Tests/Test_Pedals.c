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
    PedalsInit(); 
}

void test_UnitTestPedals_Read(void){
    for(int x = 500; x < 1000; x+=100){
        BspAdcGetMillivoltage_fake.return_val = x; 
        PedalsRead(1); 
        printf("\n\r%d\r", PedalsRead(1));
    }
}

void test_UnitTestPedals_Read_This_Will_Fail(void){
   TEST_ASSERT_EQUAL(PedalsRead(1), 30); 
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