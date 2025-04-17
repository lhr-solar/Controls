#include "common.h"
#include "os_cfg_app.h"
#include "Tasks.h"
#include <bsp.h>
#include "daybreak_pins.h"
#include "BSP_GPIO.h"
#include "StatusLeds.h"
#include "Dash.h"

int main()
{
    Status_Leds_Init();
    dashInit();
    dashboard_t dash = {0, 0, 0, 0, 0, 0};
    while (1) {
        for (uint8_t i = 0; i < NUM_DASH_PINS; i++) {
          switch(i) {
                case(BPS):
                    dash.BPS_HZD = getDash(i);
                    break;

                case(BRAKE_LED):
                    dash.BRAKE = getDash(i);
                    break;

                case(GEAR):
                    dash.GEAR = getDash(i);
                    break;

                case(HBT):
                    dash.HBT = getDash(i);
                    break;

                case(CRUZ_SET):
                    dash.CRUZ_SET = getDash(i);
                    break;

                case(CRUZ_EN):
                    dash.CRUZ_EN = getDash(i);
                    break;

                default:
                    break;
        }   

        if(dash.CRUZ_EN){
            Status_Leds_Toggle(CRUISE_IND_LED);
        }
        if(dash.CRUZ_SET){
            Status_Leds_Toggle(CRUISE_IND_LED);
        }
        if(dash.GEAR == FWD){
            Status_Leds_Write(BPS_FAULT_LED, true);
        } else if(dash.GEAR == REV){
            Status_Leds_Write(CONTROLS_FAULT_LED, true);
        } else {
            Status_Leds_Write(BPS_FAULT_LED, false);
            Status_Leds_Write(CONTROLS_FAULT_LED, false);
        }
    }
}
}