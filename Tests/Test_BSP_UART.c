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
#include <bsp.h>

#define TX_SIZE 128

int main(void) {
    BSP_UART_Init(UART_1);
    BSP_UART_Init(UART_2);
    while (1) {
        float speed = (rand() % 500) / 10.0;
        int cruiseEn = rand() % 2;
        int cruiseSet = rand() % 2;
        int regenEn = rand() % 2;
        int CANerr = rand() % 10;
        char str[TX_SIZE];
        sprintf(str, "%f, %d, %d, %d, %d", speed, cruiseEn, cruiseSet, regenEn, CANerr);

        BSP_UART_Write(UART_1, str , TX_SIZE);

        char out[2][TX_SIZE];
        BSP_UART_Read(UART_1, out[UART_1]);
        BSP_UART_Read(UART_2, out[UART_2]);
        out[UART_1][strlen(out[UART_1])-1] = 0; // remove the newline, so we can print both in one line for now
        out[UART_2][strlen(out[UART_2])-1] = 0;
        printf("UART 1: %s\tUART 2: %s\r", out[UART_1], out[UART_2]);
        /*
         * If a long message is sent before a short message, the messages will overlap
         * on the display. This is not an issue with UART_2, but just a consequence of
         * how these tests must be structured and outputted.
         */
    }
}
