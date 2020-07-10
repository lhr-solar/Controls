/**
 * Test file for library to interact with hardware connected to ADC
 * 
 * Run this test in conjunction with the simulator 
 * GUI. As you move the accelerator and brake on the GUI, the respective
 * pressed/slided percentage will change from '0' to '100' on the terminal and display
 * to show that sliding the pedals is read by the BSP
 * 
 * Uncomment the "individual tests for each function" to determine the output of individual functions
 * of the ADC module
 */ 

#include "common.h"
#include "config.h"
#include "BSP_ADC.h"
#include <bsp.h>

int main() {
    BSP_ADC_Init();
    /* INDIVUDAL TESTS FOR EACH FUNCTION
    printf("TESTS\n");

    printf("TESTING VALUE AND MILLIVOLTAGE\n");
    printf("ADC at channel of Accelerator is %d\n", BSP_ADC_Get_Value(Accelerator));
    printf("ADC at channel of Brake is %d\n", BSP_ADC_Get_Value(Brake));

    printf("ADC at channel of Accelerator in mV is %d\n", BSP_ADC_Get_Millivoltage(Accelerator));
    printf("ADC at channel of Brake in mV is %d\n", BSP_ADC_Get_Millivoltage(Brake));
    */
    while(1) {
        printf("Accelerator: %5.1d\tBrake: %5.1d\r", 
            BSP_ADC_Get_Value(ACCELERATOR), BSP_ADC_Get_Value(BRAKE));
    }
}
