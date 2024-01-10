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

DEFINE_FFF_GLOBALS;


/*** CAN Messages ***/
// static CANDATA_t bps_trip_msg = {.ID=BPS_TRIP, .idx=0, .data={1}};
// static CANDATA_t supp_voltage_msg = {.ID=SUPPLEMENTAL_VOLTAGE, .idx=0, .data={100}};
// static CANDATA_t state_of_charge_msg = {.ID=STATE_OF_CHARGE, .idx=0, .data={0}};
// static CANDATA_t HV_Array_Msg = {.ID=BPS_CONTACTOR, .idx=0, .data={0b001}};
// static CANDATA_t HV_Disable_Msg = {.ID=BPS_CONTACTOR, .idx=0, .data={0b000}};
// static CANDATA_t HV_MC_Msg = {.ID=BPS_CONTACTOR, .idx=0, .data={0b110}};
// static CANDATA_t HV_Enabled_Msg = {.ID=BPS_CONTACTOR, .idx=0, .data={0b111}};

void setUp(void) {}

void tearDown(void) {}

void test_UnitTest_bsptrip(void){
    CANbus_Read_fake.return_val = SUCCESS;
   // CANbus_Read_fake.arg0_val = bps_trip_msg;

}



/*=======MAIN=====*/
int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_UnitTest_bsptrip);
  return UNITY_END();
}