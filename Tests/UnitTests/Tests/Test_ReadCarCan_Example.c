/**
 * 
 *@copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
  * @file Test_ReadCarCan.c
 * @brief Additional unit testing example for ReadCanCan. 
 * While not a full test, it introduces a couple test strategies
 * Thing tested (if test were complete): ReadCarCan
 * Mocks used:CanBus, UpdateDisplay (though all things not tested are still mocked)
 * Note: not a real test, so we don't really mind that it fails
*/
// Unity required for this testing format
#include "unity.h"

// The header file we are unit testing:
#include "ReadCarCan.h"

// fff header so we can DEFINE_FFF_GLOBALS
// (can only happen once per test)
#include "fff.h"

// Mock headers
#include "UpdateDisplay.h"
#include "CanBus.h"

DEFINE_FFF_GLOBALS;


/*** CAN Messages ***/
static CanData bps_trip_msg = {.id=kBpsTrip, .idx=0, .data={1}};
//static CANDATA_t supp_voltage_msg = {.ID=SUPPLEMENTAL_VOLTAGE, .idx=0, .data={100}};
// static CANDATA_t state_of_charge_msg = {.ID=STATE_OF_CHARGE, .idx=0, .data={0}};
// static CANDATA_t HV_Array_Msg = {.ID=BPS_CONTACTOR, .idx=0, .data={0b001}};
static CanData HV_Disable_Msg = {.id=kBpsContactor, .idx=0, .data={0b000}};
// static CANDATA_t HV_MC_Msg = {.ID=BPS_CONTACTOR, .idx=0, .data={0b110}};
// static CANDATA_t HV_Enabled_Msg = {.ID=BPS_CONTACTOR, .idx=0, .data={0b111}};

void setUp(void) {
    FFF_RESET_HISTORY();
}

void tearDown(void) {}

// We can use a custom fake to dictate what happens when our mock function is called
// A default mock doesn't do anything, but defining a custom mock lets us do things
// like store a message in a CanData argument to simulate a message being read
ErrorStatus CANbus_Custom_fake_bps(CanData* data, bool blocking, Can bus){
  *data = bps_trip_msg;
  return SUCCESS;
}

ErrorStatus CANbus_Custom_fake_bpscontactor(CanData* data, bool blocking, Can bus){
  *data = HV_Disable_Msg;
  return SUCCESS;
}

void test_UnitTest_bsptrip(void){
    TaskInit(NULL);
    CanBusRead_fake.custom_fake = CANbus_Custom_fake_bps; // Set CanBusRead to use our custom fake
    TaskReadCarCan(NULL);
    printf("\n\r%d\n\r", CanBusRead_fake.call_count);
  
    TEST_ASSERT_EQUAL(3, ThrowTaskError_fake.arg0_history[0]);
    TEST_ASSERT_EQUAL(4, ThrowTaskError_fake.arg0_history[1]);
    printf("%d", ThrowTaskError_fake.call_count);
    TEST_ASSERT_EQUAL(false, UpdateDisplaySetArray_fake.arg0_history[0]);
    TEST_ASSERT_EQUAL(2, DisplayEvac_fake.call_count);
}

void test_UnitTest_bspcontactor(void){
    TaskInit(NULL);
    CanBusRead_fake.custom_fake = CANbus_Custom_fake_bpscontactor; // Set CanBusRead to a different custom fake
    TaskReadCarCan(NULL);
    printf("\n\r%d\n\r", CanBusRead_fake.call_count);
    
    TEST_ASSERT_EQUAL(3, ThrowTaskError_fake.arg0_history[0]);
    TEST_ASSERT_EQUAL(4, ThrowTaskError_fake.arg0_history[1]);
    printf("%d", ThrowTaskError_fake.call_count);
    TEST_ASSERT_EQUAL(false, UpdateDisplaySetArray_fake.arg0_history[0]);
}


/*=======MAIN=====*/
int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_UnitTest_bspcontactor);
  RUN_TEST(test_UnitTest_bsptrip);
  return UNITY_END();
}
