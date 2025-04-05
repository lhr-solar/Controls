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
#include "BSP_GPIO.h"
#include <bsp.h>


int main() {
    Pedals_Init();
    // Status_Leds_Init();
    BSP_UART_Init(USB);

    while(1) {
        // Status LEDs
        // for(int i = 0; i < NUM_STATUS_LED; i++){
        //     Status_Leds_Toggle(i);
        //     volatile uint32_t waitTime = 0;
        //     while (waitTime <= 99999) waitTime++;
        // }

        // Actual Test
        printf("Accelerator: %5.1d%%\tBrake: %5.1d%%\n\r", 
        Pedals_Read(ACCELERATOR),Pedals_Read(BRAKE));
        printf("BSP Get Millivoltage Accel: %5.1dmV\tBSP Get Millivoltage Brake:%5.1dmV\n\r", 
            BSP_ADC_Get_Millivoltage(Accelerator_ADC),BSP_ADC_Get_Millivoltage(Brake_ADC));

        // Uncomment if testing ADC/DMA
        volatile uint16_t *ADCresults_test = getADCResults();
        printf("ADC Results Entry Accelerator_ADC: %5.1d\n\r", ADCresults_test[Accelerator_ADC] );
        printf("ADC Results Entry Brake_ADC: %5.1d\n\r", ADCresults_test[Brake_ADC] );
        printf("ADC Results Entry Extra1: %5.1d\n\r", ADCresults_test[Extra1] );
        printf("ADC Results Entry Extra2: %5.1d\n\r", ADCresults_test[Extra2] );

        for(volatile int i = 0; i < 150000; i++) {
        }
    }
}

