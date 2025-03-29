/**
 * Test file for library to interact with the pedal driver file
 * 
 * Run this test in conjunction with the simulator 
 * GUI. As you move the accelerator and brake on the GUI, the respective
 * pressed/slided percentage will change from '0' to '100' on the terminal and display
 * to show that sliding the pedals is read by the BSP
 * 
 */ 

#include "common.h"
#include "config.h"
#include "Pedals.h"
#include "StatusLeds.h"
#include <bsp.h>


int main() {
    Pedals_Init();
    Status_Leds_Init();
    BSP_UART_Init(USB);

    // while(1) {
    //     Status_Leds_Toggle(CRUISE_IND_LED);
    //     for(int i = 0; i < 100; i++) {}
    // }
    

    while(1) {
        printf("Brake Percent: %d\n\r", (int16_t) BSP_ADC_Get_Millivoltage(BRAKE));
        // printf("Brake Percent: %d\n\r", Pedals_Read(BRAKE));
        printf("Brake Percent: %d\n\r", Pedals_Read(BRAKE));
        for(int i = 0; i < 500000; i++){}
    }

    // while(1) {
    //     printf("Accelerator: %5.1d%%\tBrake: %5.1d%%\r", 
    //         Pedals_Read(ACCELERATOR),Pedals_Read(BRAKE));
    // }
}

