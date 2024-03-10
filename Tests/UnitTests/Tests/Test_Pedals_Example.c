/**
 * 
 *@copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
  * @file Test_Pedals.c
 * @brief An example containing the basic format of a unit test. 
 * Though not a functional test, it still contains some major elements
 * that are necessary in any unit test.
 * Thing tested (if test were complete): Pedals functions
 * Mocks used:BSP functions (though all things not tested are still mocked)
*/

// Unity: the unit testing framework we are using
// Unity contains the assertions required for this testing format
#include "unity.h" 

// The header file we are unit testing
#include "Pedals.h" 

// Fake Function Framework: helps us generate mocks and store data
// We need the header here for DEFINE_FFF_GLOBALS
#include "fff.h"

// This defines global variables used for test data
// (and therefore can't be defined multiple times in a build)
DEFINE_FFF_GLOBALS;

// Special function from Unity that runs before each test
void setUp(void) {}

// Special function from Unity that runs after each test
void tearDown(void) {}

// A test for PedalsInit which we wrote ourselves and which gets called in main
// This example calls PedalsInit but doesn't actually verify anthing
void test_UnitTestPedalsInit(void){
    PedalsInit(); 
}

// Another test, this time for PedalsRead, which just demos using fake functions
void test_UnitTestPedalsRead(void){
    for(int x = 500; x < 1000; x+=100){

        // We have defined mock functions (found in the Mocks folder) using fff
        // When we unit test, we include the mock functions in place of the real ones
        // One of the things we can do with mocks is set their return value
        BspAdcGetMillivoltage_fake.return_val = x;
       // Now whenever BspAdcGetMillivoltage is called, the mock will return x

        PedalsRead(1); 
        printf("\n\r%d\r", PedalsRead(1));
    }
}

// A test that will always fail
void test_UnitTestPedals_Read_This_Will_Fail(void){
    // Unity gives us asserts to verify values
    // When an assert fails, Unity will tell us our test has failed
    TEST_ASSERT_EQUAL(PedalsRead(1), 30); 
}



/*=======MAIN=====*/
// Here we use Unity's framework to run the tests we've written
int main(void)
{
  UNITY_BEGIN(); 
  RUN_TEST(test_UnitTestPedalsInit); 
  RUN_TEST(test_UnitTestPedalsRead);
  RUN_TEST(test_UnitTestPedals_Read_This_Will_Fail); 
  return UNITY_END(); 
}