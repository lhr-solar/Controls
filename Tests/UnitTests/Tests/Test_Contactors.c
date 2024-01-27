/**
 * @copyright Copyright (c) 2018-2024 UT Longhorn Racing Solar
 * @file Test_Contactors.c
 * @brief An annotated example unit test written for the driver Contactors.c
 * 
 */

#include "unity.h" // Contains assertions for testing
#include "Contactors.h" // Header of the source file to test
#include "os.h"

// Global variables for FFF; must be included once per unit-test file
DEFINE_FFF_GLOBALS; 

// Defines for the test
#define BOOL_STATES 2


// Special function required by Unity that will run before each test
void setUp(void){
    // Reset the fakes' call and argument history between tests
    RESET_FAKE(BspGpioWritePin) // not really needed
    RESET_FAKE(BspGpioInit) // technically not needed since we only use this in one test
    RESET_FAKE(BspGpioGetState) // same here
    RESET_FAKE(OSMutexPend) // ""
    RESET_FAKE(OSMutexPost) // ""
    // Would like to make this part less tedious but also you'll need to deal with all of these anyways    
}


// Another required function from Unity that will run after each test
void tearDown(void){
    // Left blank because we don't need it
}


// Helper functions
static uint32_t count_instances_int(int match, int* array, uint32_t numElems) {
    uint32_t num = 0;
    for (uint16_t i = 0; i < numElems; i++) {
        if (array[i] == match) {
            num++;
        }
    }
    return num;
}

static uint32_t count_instances_uint16(uint16_t match, uint16_t* array, uint32_t numElems) {
    uint32_t num = 0;
    for (uint16_t i = 0; i < numElems; i++) {
        if (array[i] == match) {
            num++;
        }
    }
    return num;
}


static uint32_t count_instances_bool(bool match, bool* array, uint32_t numElems) {
    uint32_t num = 0;
    for (uint16_t i = 0; i < numElems; i++) {
        if (array[i] == match) {
            num++;
        }
    }
    return num;
}

// Tests
void test_ContactorsInit(){
    ContactorsInit();
    // Check that BspGpioInit was called to initialize
    // Contactors port for Array and Motor Precharge pins to be outputs
    TEST_ASSERT_EQUAL(1, BspGpioInit_fake.call_count);
    TEST_ASSERT_EQUAL(CONTACTORS_PORT, BspGpioInit_fake.arg0_val);
    TEST_ASSERT_EQUAL((ARRAY_PRECHARGE_BYPASS_PIN | MOTOR_CONTROLLER_PRECHARGE_BYPASS_PIN), BspGpioInit_fake.arg1_val);
    TEST_ASSERT_EQUAL (1, BspGpioInit_fake.arg2_val);

    // Check that all pins were set to OFF
    TEST_ASSERT_EQUAL(BspGpioWritePin_fake.call_count, kNumContactors);
    TEST_ASSERT_EQUAL(kNumContactors, count_instances_int(CONTACTORS_PORT, (int *)BspGpioWritePin_fake.arg0_history, kNumContactors));
    TEST_ASSERT_EQUAL(1, count_instances_uint16(ARRAY_PRECHARGE_BYPASS_PIN, BspGpioWritePin_fake.arg1_history, kNumContactors));
    TEST_ASSERT_EQUAL(1, count_instances_uint16(MOTOR_CONTROLLER_PRECHARGE_BYPASS_PIN, BspGpioWritePin_fake.arg1_history, kNumContactors));
    TEST_ASSERT_EQUAL(2, count_instances_bool(OFF, (bool *)BspGpioWritePin_fake.arg2_history, kNumContactors));
}

void test_ContactorsGet(){
    // When BspGpio_Read is called with a certain pin, we want to make sure ContactorsGet returns the right thing
    BspGpioGetState_fake.return_val = ON; // Test array and motor contactor when ON
    TEST_ASSERT_EQUAL(ON, ContactorsGet(kArrayPrechargeBypassContactor));
    TEST_ASSERT_EQUAL(ON, ContactorsGet(kMotorControllerPrechargeBypassContactor));

    BspGpioGetState_fake.return_val = OFF; // Test array and motor contactor when OFF
    TEST_ASSERT_EQUAL(OFF, ContactorsGet(kArrayPrechargeBypassContactor));
    TEST_ASSERT_EQUAL(OFF, ContactorsGet(kMotorControllerPrechargeBypassContactor));
    
    TEST_ASSERT_EQUAL(4, BspGpioGetState_fake.call_count);
}

void test_ContactorsSet(){

    int mutex_calls = 0;

    // Test all input combinations
    for (uint8_t contactor = 0; contactor < kNumContactors; contactor++) {
        for (uint8_t state = 0; state < BOOL_STATES; state++) { 
            for (uint8_t blocking = 0; blocking < BOOL_STATES; blocking++) {
                
                uint16_t pin_mask = 0;
                
                if (contactor == kArrayPrechargeBypassContactor) {
                    pin_mask = ARRAY_PRECHARGE_BYPASS_PIN;
                } else if (contactor == kMotorControllerPrechargeBypassContactor) {
                    pin_mask = MOTOR_CONTROLLER_PRECHARGE_BYPASS_PIN;
                }
                
                ContactorsSet((Contactor)contactor, (bool)state, (bool)blocking);
                TEST_ASSERT_EQUAL(CONTACTORS_PORT, BspGpioWritePin_fake.arg0_val);
                TEST_ASSERT_EQUAL(pin_mask, BspGpioWritePin_fake.arg1_val);
                TEST_ASSERT_EQUAL(state, BspGpioWritePin_fake.arg2_val);

                if (blocking){
                    TEST_ASSERT_EQUAL(OS_OPT_PEND_BLOCKING, OSMutexPend_fake.arg1_val);    
                } else {
                    TEST_ASSERT_EQUAL(OS_OPT_PEND_BLOCKING, OSMutexPend_fake.arg1_val);
                }

                mutex_calls++;
                TEST_ASSERT_EQUAL(mutex_calls, OSMutexPend_fake.call_count);
                TEST_ASSERT_EQUAL(mutex_calls, OSMutexPost_fake.call_count);
                
            }
        }
    }
    
    TEST_ASSERT_EQUAL(kNumContactors * BOOL_STATES * BOOL_STATES, BspGpioWritePin_fake.call_count);
}

// Runs the tests
int main(void) {
    UNITY_BEGIN(); // UNITY_BEGIN and UNITY_END are required
    RUN_TEST(test_ContactorsInit);
    RUN_TEST(test_ContactorsGet);
    RUN_TEST(test_ContactorsSet);
  return UNITY_END();
}