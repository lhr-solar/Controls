/**
 * @copyright Copyright (c) 2018-2024 UT Longhorn Racing Solar
 * @file Test_Contactors.c
 * @brief An annotated example unit test written for the driver Contactors.c
 * 
 */

#include "unity.h" // Contains assertions for testing
#include "Contactors.h" // Header of the source file to test
//#include "fff.h" // Fake Function Framework for working with mocks

// Global variables for FFF; must be included once per unit-test file
DEFINE_FFF_GLOBALS; 


// Special function required by Unity that will run before each test
void setUp(void){
    // Reset the fakes' call and argument history between tests
    RESET_FAKE(BSP_GPIO_Write_Pin) // not really needed
    RESET_FAKE(BSP_GPIO_Init) // technically not needed since we only use this in one test
    RESET_FAKE(OSMutexCreate) // ""
    RESET_FAKE(BSP_GPIO_Get_State) // same here
    RESET_FAKE(OSMutexPend) //""
    RESET_FAKE(OSMutexPost) // ""
    // Would like to make this part less tedious but also you'll need to deal with all of these anyways    
}


// Another required function from Unity that will run after each test
void tearDown(void){
    // Left blank because we don't need it
}


void test_Contactors_Init(){
    // Check that BSP_GPIO_Init was called to initialize
    // Contactors port for Array and Motor Precharge pins to be outputs
    TEST_ASSERT_EQUAL(BSP_GPIO_Init_fake.arg0_history[0], CONTACTORS_PORT)
    TEST_ASSERT_EQUAL(BSP_GPIO_Init_fake.arg1_history[0], (ARRAY_PRECHARGE_BYPASS_PIN | MOTOR_CONTROLLER_PRECHARGE_BYPASS_PIN))
    TEST_ASSERT_EQUAL (BSP_GPIO_Init_fake.arg2_history[0], (1))

    // umm set contactor says it should only be called if a mutex is held...?
    // also I don't know how to test the order in which functions were called


    // Check that all contactors were set to "off"
    // for (int contactor = 0; contactor < NUM_CONTACTORS; ++contactor) {
    //     TEST_ASSERT_EQUAL(BSP_GPIO_Write_Pin_fake.arg0_history[0], CONTACTORS_PORT)
    //     TEST_ASSERT_EQUAL(BSP_GPIO_Write_Pin_fake.arg1_history[0], ARRAY_PRECHARGE_BYPASS_PIN)
    // }
    TEST_ASSERT_EQUAL(BSP_GPIO_Write_Pin_fake.arg0_history[0], CONTACTORS_PORT)
    TEST_ASSERT_EQUAL(BSP_GPIO_Write_Pin_fake.arg1_history[0], ARRAY_PRECHARGE_BYPASS_PIN)
    TEST_ASSERT_EQUAL(BSP_GPIO_Write_Pin_fake.arg2_history[0], OFF) 

    BSP_GPIO_Turn_Array_Off_Args = {CONTACTORS_PORT, ARRAY_PRECHARGE_BYPASS_PIN, OFF}
    for (int i = 0; i < 3; i++) { // number of arguments
        //hmmm #define FUNC_PARAM BSP_GPIO_Write_Pin_fake.arg##i##_history[0]
        TEST_ASSERT_EQUAL(FUNC_PARAM, BSP_GPIO_Turn_Array_Off_Args[i])
    }  
}

