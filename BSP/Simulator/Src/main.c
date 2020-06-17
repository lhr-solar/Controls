#include "ADC.h"
#include <stdio.h>

int main(void){
    printf("ADC value %d\n", getADCvalue());
    printf(" ADC percentage: %f\n", getADCpercentage());
    return 1;
}