#include "common.h"
#include "StatusLeds.h"
#include "Dashboard.h"

int main()
{
    BSP_UART_Init(USB);
    Status_Leds_Init();
    dashboardInit();
    gear_t gear = GEAR_FAULT_ERROR;
    //uint8_t dashStatus[NUM_DASH_PINS];


    //// Testing forced gear reset to neutral
    // Move to non-neutral start point
    printf("ACTION: Please move the gear switch to FWD gear\n\r");
    while(BSP_GPIO_Read_Pin(REVERSE_PORT, REVERSE) || !BSP_GPIO_Read_Pin(FORWARD_PORT, FORWARD)) {}
    while(getGear() != NEU) {} // Should be manually overriden to neutral
    // Move to neutral to enable reset
    printf("ACTION: Please move the gear switch to NEU gear\n\r");
    while(BSP_GPIO_Read_Pin(REVERSE_PORT, REVERSE) || BSP_GPIO_Read_Pin(FORWARD_PORT, FORWARD)) {}
    while(getGear() != NEU) {}
    // Move to forward now that reset logic should be done
    printf("ACTION: Please move the gear switch to FWD gear\n\r");
    while(getGear() != FWD) {} // Should operate as expected now
    printf("SUCCESS!! Forced gear reset to neutral works\n\r");

    
    while (1) {

        // for(int i = 0; i < NUM_DASH_PINS; i++){
        //     dashStatus[i] = getDashState(i);
        // }

        Status_Leds_All_Off();
        Status_Leds_Write(DASH_BPS_HAZ_LED, true);

        if(getSwitchState(CRUZ_EN)){
            Status_Leds_Write(CRUISE_IND_LED, true);    //light up cruise indicator on leader 
        }                                               //for cruise buttons
        if(getSwitchState(CRUZ_SET)){
            Status_Leds_Write(CRUISE_IND_LED, true);
        }

        gear = getGear();
        if(gear == FWD){                                   //light up stuff for FWD/REV
            Status_Leds_Write(BPS_FAULT_LED, true);
        } else if(gear == REV){
            Status_Leds_Write(CONTROLS_FAULT_LED, true);
        } else {
            Status_Leds_Write(BPS_FAULT_LED, false);
            Status_Leds_Write(CONTROLS_FAULT_LED, false);
        }
    }
    Status_Leds_Write(DASH_BPS_HAZ_LED, false); 
}