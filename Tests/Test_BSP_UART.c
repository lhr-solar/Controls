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
    // BSP_UART_Init(USB);
    BSP_UART_Init(DISPLAY);
    const char* testStr = "\xff\xff\xffpage 2\xff\xff\xff";
    BSP_UART_Write(DISPLAY, (char*) testStr , strlen(testStr));
    while (1) {volatile int x=0; x++;}

        // char out[2][TX_SIZE];
        // BSP_UART_Read(USB, out[USB]);
        // BSP_UART_Read(DISPLAY, out[DISPLAY]);
        // out[USB][strlen(out[USB])-1] = 0; // remove the newline, so we can print both in one line for now
        // out[DISPLAY][strlen(out[DISPLAY])-1] = 0;
        // printf("UART 2: %s\tUART 3: %s\r", out[USB], out[DISPLAY]);
        // /*
        //  * If a long message is sent before a short message, the messages will overlap
        //  * on the display. This is not an issue with USB, but just a consequence of
        //  * how these tests must be structured and outputted.
        //  */
}
