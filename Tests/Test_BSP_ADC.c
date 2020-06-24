#include "common.h"
#include "config.h"
#include "BSP_ADC.h"

int main() {
    ADC_Init();
    while(1) {
        printf("Accelerator: %5.1f\tBrake: %5.1f\r", 
            ADC_Percent_Accel_Read(), ADC_Percent_Brake_Read());
    }
}
