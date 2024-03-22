/**
 * Test file for ADC
 * Two channels: accelerator and brake
 * Simply turn on the board and connect serial (UART2) as well as the potentiometer or
 * analog device to the ADC pins, to inspect the values
 */ 

#include "common.h"
#include "config.h"
#include "BSP_ADC.h"
#include "BSP_UART.h"
#include <bsp.h>

int main() {
    BSP_ADC_Init();
    BSP_UART_Init(UART_2);

    while(1) {
        printf("Accelerator: %5.1d\tBrake: %5.1d\r", 
            BSP_ADC_Get_Millivoltage(Accelerator_ADC), BSP_ADC_Get_Millivoltage(Brake_ADC));
    }
}
