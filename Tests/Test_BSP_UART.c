/**
 * Test file for library to interact with UART
 * 
 * Run this test in conjunction with the simulator
 * GUI. As this generates randomized values, the display
 * will update the values accordingly to show that the
 * display is reading the UART properly
 */

#include "common.h"
#include "config.h"
#include "BSP_UART.h"

#define TX_SIZE 128

int main(void) {
    BSP_UART_Init();
    while (1) {
        float speed = (rand() % 500) / 10.0;
        int cruiseEn = rand() % 2;
        int cruiseSet = rand() % 2;
        int regenEn = rand() % 2;
        int CANerr = rand() % 10;
        char str[TX_SIZE];
        sprintf(str, "%f, %d, %d, %d, %d", speed, cruiseEn, cruiseSet, regenEn, CANerr);

        BSP_UART_Write(str, TX_SIZE);
    }
}
