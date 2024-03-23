/**
 * Test file for library to interact with the pedal driver file
 * 
 * Reads the Acceleration and Brake signals and prints them out
 * 
 */ 

#include "Pedals.h"

int main(void) {
    Pedals_Init();

    while(1) {
        printf("Accelerator: %5.1d%%\tBrake: %5.1d%%\n\r", 
            Pedals_Read(ACCELERATOR), Pedals_Read(BRAKE));
    }
}
