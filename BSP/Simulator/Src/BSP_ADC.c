#include <stdio.h>
#include <stdlib.h>
#include "BSP_ADC.h"
#include "common.h"
#include "config.h"

/*
C file that reads the raw ADC data from the CSV file, 
converts it back to a distance pressed (percentage for now), and stores both.
We should be able to request both the raw ADC data 
and the distance (percentage for now) from this library.
 */

//NOTE: ADC precision is 12 bits (4096), range 3.3V
//I'm assuming 3.3V corresponds to a 100% pressed pedal

#define FILE_NAME "BSP/Simulator/Hardware/Data/Pedals.csv"

int ADCvalueAccel, ADCValueBrake;
float ADCPercentageAccel, ADCPercentageBrake;

void ADC_Init(void) {
    // TODO
}

//helper function that returns percentage value of ADC value 
float convert_ADC_to_Percent(int ADCvalue) {
    return (((float)ADCvalue) / 4096) * 100;
}

void calculate_ADC_Values(void) {
    //opening file in read mode
    FILE *filePointer = fopen(FILE_NAME, "r");
    //scanning content in the file and storing value in ADCvalue int
    fscanf(filePointer, "%d,%d", &ADCvalueAccel, &ADCValueBrake);
    //closing the file
    fclose(filePointer);
    //convert ADC value to a percentage using a helper function
    ADCPercentageAccel = convert_ADC_to_Percent(ADCvalueAccel);
    ADCPercentageBrake = convert_ADC_to_Percent(ADCValueBrake);
    //printing raw ADC value and pressed percentage
    //printf("ADC value: %d, pressed percentage: %f", ADCvalue, ADCpercentage);
}

float ADC_Percent_Accel_Read() {
    calculate_ADC_Values();
    return ADCPercentageAccel;
}

float ADC_Percent_Brake_Read() {
    calculate_ADC_Values();
    return ADCPercentageBrake;
}
