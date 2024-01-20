//////////////////////////////////////////
//////////////////////////////////////////
// Unity required for this testing format
#include "unity.h"
//////////////////////////////////////////
//////////////////////////////////////////
// The header file we are unit testing:
#include "SendTritium.h"
//////////////////////////////////////////
// fff header so we can DEFINE_FFF_GLOBALS
// (can only happen once per test)
#include "fff.h"
/////////////////////
// Mock headers
#include "UpdateDisplay.h"

DEFINE_FFF_GLOBALS;



void setUp(void) {
    FFF_RESET_HISTORY();
}

void tearDown(void) {}

ErrorStatus CANbus_Custom_(CANDATA_t* data, bool blocking, CAN_t bus){
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
