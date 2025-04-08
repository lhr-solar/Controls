/**
 * Test file for library to interact with the pedal driver file
 * 
 * Run this test in conjunction with the simulator 
 * GUI. As you move the accelerator and brake on the GUI, the respective
 * pressed/slided percentage will change from '0' to '100' on the terminal and display
 * to show that sliding the pedals is read by the BSP
 * 
 */ 

#include "common.h"
#include "config.h"
#include "Pedals.h"
#include <bsp.h>


int main() {
    Pedals_Init();
    BSP_UART_Init(UART_2);

    while(1) {
        for(int i = 0; i < 99999; i++) {
            if(i == 0) {
                printf("Accelerator: %5.1d%%\tBrake: %5.1d%%\n\r", 
                Pedals_Read(ACCELERATOR),Pedals_Read(BRAKE));
            }
        }
    }
}
