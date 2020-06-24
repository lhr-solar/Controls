#ifndef __BSP_ADC_H
#define __BSP_ADC_H

//function that returns the percentage pressed of the accelerator pedal
float ADC_Percent_Accel_Read();

//function that returns the percentage pressed of the accelerator pedal
float ADC_Percent_Brake_Read();

// Initialize the ADC module
void ADC_Init(void);

#endif