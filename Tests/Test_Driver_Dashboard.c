#include "common.h"
#include "StatusLeds.h"
#include "Dashboard.h"

int main()
{
    Status_Leds_Init();
    dashboardInit();
    switch_state state = SWITCH_ERROR;
    //uint8_t dashStatus[NUM_DASH_PINS];
    while (1) {

        // for(int i = 0; i < NUM_DASH_PINS; i++){
        //     dashStatus[i] = getDashState(i);
        // }

        Status_Leds_All_Off();
        Status_Leds_Write(DASH_BPS_HAZ_LED, true);

        if(getDashState(CRUZ_EN)){
            Status_Leds_Write(CRUISE_IND_LED, true);    //light up cruise indicator on leader 
        }                                               //for cruise buttons
        if(getDashState(CRUZ_SET)){
            Status_Leds_Write(CRUISE_IND_LED, true);
        }

        state = getDashState(GEAR);
        if(state == FWD){                                   //light up stuff for FWD/REV
            Status_Leds_Write(BPS_FAULT_LED, true);
        } else if(state == REV){
            Status_Leds_Write(CONTROLS_FAULT_LED, true);
        } else {
            Status_Leds_Write(BPS_FAULT_LED, false);
            Status_Leds_Write(CONTROLS_FAULT_LED, false);
        }
    }
    Status_Leds_Write(DASH_BPS_HAZ_LED, false); 
}