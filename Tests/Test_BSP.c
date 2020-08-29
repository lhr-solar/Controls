/**
 * Test file that unifies all individual test files of the BSP module
 * 
 * Run this test in conjunction with the simulator 
 * GUI. 
 * 
 */ 

#include "common.h"
#include "config.h"
#include <unistd.h>
#include "BSP_Timer.h"
#include "BSP_ADC.h"
#include <bsp.h>


int main() {
    while(1) {
        
        printf("----------------------------------------------------\n");
        printf("--------------------ADC Test------------------------\n");
        //Init the BSP with the A
        BSP_ADC_Init(ADC_0);
        BSP_ADC_Init(ADC_1);

        BSP_CAN_Init(CAN_1);
        BSP_CAN_Init(CAN_2);
        printf("Accelerator: %5.1d\tBrake: %5.1d\r", BSP_ADC_Get_Millivoltage(ADC_0), BSP_ADC_Get_Millivoltage(ADC_1));

    }
}
