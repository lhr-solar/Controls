#include "common.h"
#include "StatusLeds.h"
#include "Dashboard.h"

int main()
{
    Status_Leds_Init();
    dashboardInit();
    switch_state state = SWITCH_ERROR;
    while (1) {
        Status_Leds_All_Off();
        Status_Leds_Write(DASH_BPS_HAZ_LED, true);
        if(getDashState(CRUZ_EN)){
            Status_Leds_Write(CRUISE_IND_LED, true);
        }
        if(getDashState(CRUZ_SET)){
            Status_Leds_Write(CRUISE_IND_LED, true);
        }
        state = getDashState(GEAR);
        if(state == FWD){
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