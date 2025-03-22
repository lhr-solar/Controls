#include "StatusLeds.h"
#include "Ignition.h"

int main(void){
    Status_Leds_Init();
    Ignition_Init();
    Status_Leds_Write(BPS_FAULT_LED, ON);
    
    while(1){
        ignition_state_t state = Get_Ignition_State();
        switch(state){
            case IGN_OFF:
                Status_Leds_Write(OS_FAULT_LED, ON);
                break;
            case IGN_ARR:
                Status_Leds_Write(CRUISE_IND_LED, ON);
                break;
            case IGN_MOTOR:
                Status_Leds_Write(MOTOR_CONTACTOR_LED, ON);
                break;
            case IGN_ERROR:
                Status_Leds_Write(CONTROLS_FAULT_LED, ON);
                break;
        }
        Status_Leds_Write(OS_FAULT_LED, OFF);
        Status_Leds_Write(CRUISE_IND_LED, OFF);
        Status_Leds_Write(MOTOR_CONTACTOR_LED, OFF);
        Status_Leds_Write(CONTROLS_FAULT_LED, OFF);
    }
}