#include <stdio.h>
#include <stdlib.h>
#include "ADC.h"

/*
C file that reads the raw ADC data from the CSV file, 
converts it back to a distance pressed (percentage for now), and stores both.
We should be able to request both the raw ADC data 
and the distance (percentage for now) from this library.
 */

//NOTE: ADC precision is 12 bits (4096), range 3.3V
//I'm assuming 3.3V corresponds to a 100% pressed pedal

int ADCvalue;
float ADCpercentage;

//helper function that returns percentage value of ADC value 
float convertADCtoPercent(float ADCvalue){
    return (ADCvalue / 4096) * 100;
}

void calculationADCValues(void){
    //opening file in read mode
    FILE *filePointer = fopen("ADC.csv", "r");
    //scanning content in the file and storing value in ADCvalue int
    fscanf(filePointer, "%d", &ADCvalue);
    //closing the file
    fclose(filePointer);
    //convert ADC value to a percentage using a helper function
    ADCpercentage = convertADCtoPercent((float)ADCvalue);
    //printing raw ADC value and pressed percentage
    //printf("ADC value: %d, pressed percentage: %f", ADCvalue, ADCpercentage);
}

int getADCvalue(){
    calculationADCValues();
    return ADCvalue;
}

float getADCpercentage(){
    calculationADCValues();
    return ADCpercentage;
}
