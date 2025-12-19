#include "common.h"
#include "StatusLeds.h"
#include "Dashboard.h"

int main()
{
    BSP_UART_Init(USB);
    Status_Leds_Init();
    dashboardInit();
    gear_t gear = DASH_GEAR_FAULT_ERROR;
    //uint8_t dashStatus[NUM_DASH_PINS];

    Status_Leds_All_Off();

    //// Testing forced gear reset to neutral
    // NOTE: MOTOR_PRECHARGE_CONTACTOR_LED On => Move the gearswitch to FWD gear
    // NOTE: ARRAY_PRECHARGE_CONTACTOR_LED On => Move the gearswitch to NEU gear
    // NOTE: Both MOTOR_PRECHARGE_CONTACTOR_LED & ARRAY_PRECHARGE_CONTACTOR_LED on => Done testing forced gear reset to neutral, it works!

    // Move to non-neutral start point
    Status_Leds_Write(MOTOR_PRECHARGE_CONTACTOR_LED, true);
    while(BSP_GPIO_Read_Pin(REVERSE_PORT, REVERSE) || !BSP_GPIO_Read_Pin(FORWARD_PORT, FORWARD)) {}
    while(getGear() != DASH_NEU) {} // Should be manually overriden to neutral
    // Move to neutral to enable reset
    Status_Leds_Write(MOTOR_PRECHARGE_CONTACTOR_LED, false);
    Status_Leds_Write(ARRAY_PRECHARGE_CONTACTOR_LED, true);
    while(BSP_GPIO_Read_Pin(REVERSE_PORT, REVERSE) || BSP_GPIO_Read_Pin(FORWARD_PORT, FORWARD)) {}
    while(getGear() != DASH_NEU) {}
    // Move to forward now that reset logic should be done
    Status_Leds_Write(MOTOR_PRECHARGE_CONTACTOR_LED, true);
    Status_Leds_Write(ARRAY_PRECHARGE_CONTACTOR_LED, false);
    while(getGear() != DASH_FWD) {} // Should operate as expected now
    // Forced gear reset to neutral is a success!!
    Status_Leds_Write(ARRAY_PRECHARGE_CONTACTOR_LED, true);

    
    while (1) {

        // for(int i = 0; i < NUM_DASH_PINS; i++){
        //     dashStatus[i] = getDashState(i);
        // }

        Status_Leds_All_Off();
        Status_Leds_Write(DASH_BPS_HAZ_LED, true);

        if(getSwitchState(DASH_CRUZ_EN)){
            Status_Leds_Write(CRUISE_IND_LED, true);    //light up cruise indicator on leader 
        }                                               //for cruise buttons
        if(getSwitchState(DASH_CRUZ_SET)){
            Status_Leds_Write(CRUISE_IND_LED, true);
        }

        gear = getGear();
        if(gear == DASH_FWD){                                   //light up stuff for FWD/REV
            Status_Leds_Write(BPS_FAULT_LED, true);
        } else if(gear == DASH_REV){
            Status_Leds_Write(CONTROLS_FAULT_LED, true);
        } else {
            Status_Leds_Write(BPS_FAULT_LED, false);
            Status_Leds_Write(CONTROLS_FAULT_LED, false);
        }
    }
    Status_Leds_Write(DASH_BPS_HAZ_LED, false); 
}