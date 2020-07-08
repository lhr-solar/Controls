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

int main() {
    BSP_ADC_Init();
    
    printf("test, %d\n", BSP_ADC_Get_Value(Accelerator));
/* INDIVUDAL TESTS FOR EACH FUNCTION
    printf("TESTS\n");
    int numberOfChannels = BSP_ADC_Number_Values();
    printf("TESTING NUMBER OF CHANNELS FUNCTION\n");
    printf("Number of Channels: %d\n", numberOfChannels);

    printf("TESTING VALUE AND MILLIVOLTAGE\n");
    printf("ADC at channel 0 is %d\n", BSP_ADC_Get_Value(0));
    printf("ADC at channel 1 is %d\n", BSP_ADC_Get_Value(1));
    printf("ADC at channel 2 is %d\n", BSP_ADC_Get_Value(2));

    printf("ADC at channel 0 in mV is %f\n", BSP_ADC_Get_Millivoltage(0));
    printf("ADC at channel 1 in mV is %f\n", BSP_ADC_Get_Millivoltage(1));
    printf("ADC at channel 2 is mV is %f\n", BSP_ADC_Get_Millivoltage(2));

    printf("TESTING BSP_ADC_Read_Values FUNCTION\n");
    int * values = BSP_ADC_Read_Values();
    for(int i = 0; i < numberOfChannels; i++){
        printf("ADC at channel %d is %d\n", i, values[i]);
    }

    printf("TESTING ACCEL AND BRAKE FUNCTION\n");
    printf("Accelerator percentage: %f\n", BSP_ADC_Accel_GetPercentage());
    printf("Brake percentage: %f\n", BSP_ADC_Brake_GetPercentage());
    printf("Accelerator millivolts:  %f\n", BSP_ADC_Accel_GetMillivoltage());
    printf("Brake millivolts: %f\n", BSP_ADC_Brake_GetMillivoltage());
    */
    /*while(1) {
        printf("Accelerator: %5.1f\tBrake: %5.1f\r", 
            BSP_ADC_Accel_GetPercentage(), BSP_ADC_Brake_GetPercentage());
    }*/

    

}
