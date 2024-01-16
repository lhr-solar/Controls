//////////////////////////////////////////
//////////////////////////////////////////
// Unity required for this testing format
#include "unity.h"
//////////////////////////////////////////
//////////////////////////////////////////
// The header file we are unit testing:
#include "ReadCarCAN.h"
//////////////////////////////////////////
// fff header so we can DEFINE_FFF_GLOBALS
// (can only happen once per test)
#include "fff.h"
/////////////////////
// Mock headers
#include "UpdateDisplay.h"

DEFINE_FFF_GLOBALS;


/*** CAN Messages ***/
static CANDATA_t bps_trip_msg = {.ID=BPS_TRIP, .idx=0, .data={1}};
//static CANDATA_t supp_voltage_msg = {.ID=SUPPLEMENTAL_VOLTAGE, .idx=0, .data={100}};
// static CANDATA_t state_of_charge_msg = {.ID=STATE_OF_CHARGE, .idx=0, .data={0}};
// static CANDATA_t HV_Array_Msg = {.ID=BPS_CONTACTOR, .idx=0, .data={0b001}};
static CANDATA_t HV_Disable_Msg = {.ID=BPS_CONTACTOR, .idx=0, .data={0b000}};
// static CANDATA_t HV_MC_Msg = {.ID=BPS_CONTACTOR, .idx=0, .data={0b110}};
// static CANDATA_t HV_Enabled_Msg = {.ID=BPS_CONTACTOR, .idx=0, .data={0b111}};

void setUp(void) {
    FFF_RESET_HISTORY();
}

void tearDown(void) {}

ErrorStatus CANbus_Custom_fake_bps(CANDATA_t* data, bool blocking, CAN_t bus){
  *data = bps_trip_msg;
  return SUCCESS;
}

ErrorStatus CANbus_Custom_fake_bpscontactor(CANDATA_t* data, bool blocking, CAN_t bus){
  *data = HV_Disable_Msg;
  return SUCCESS;
}

void test_UnitTest_bsptrip(void){
    Task_Init(NULL);
    CANbus_Read_fake.custom_fake = CANbus_Custom_fake_bps;
    Task_ReadCarCAN(NULL);
    printf("\n\r%d\n\r", CANbus_Read_fake.call_count);
 //   
    TEST_ASSERT_EQUAL(3, throwTaskError_fake.arg0_history[0]);
    TEST_ASSERT_EQUAL(4, throwTaskError_fake.arg0_history[1]);
    printf("%d", throwTaskError_fake.call_count);
    TEST_ASSERT_EQUAL(false, UpdateDisplay_SetArray_fake.arg0_history[0]);
    TEST_ASSERT_EQUAL(2, Display_Evac_fake.call_count);
}

void test_UnitTest_bspcontactor(void){
    Task_Init(NULL);
    CANbus_Read_fake.custom_fake = CANbus_Custom_fake_bpscontactor;
    Task_ReadCarCAN(NULL);
    printf("\n\r%d\n\r", CANbus_Read_fake.call_count);
 //   
    TEST_ASSERT_EQUAL(3, throwTaskError_fake.arg0_history[0]);
    //TEST_ASSERT_EQUAL(4, throwTaskError_fake.arg0_history[1]);
    printf("%d", throwTaskError_fake.call_count);
    TEST_ASSERT_EQUAL(false, UpdateDisplay_SetArray_fake.arg0_history[0]);
}


/*=======MAIN=====*/
int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_UnitTest_bspcontactor);
  RUN_TEST(test_UnitTest_bsptrip);
  return UNITY_END();
}
