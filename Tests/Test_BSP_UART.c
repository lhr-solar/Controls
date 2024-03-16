/**
 * Test file for library to interact with UART
 * 
 * 
 */

#include "common.h"
#include "config.h"
#include <bsp.h>

#define TX_SIZE 128

void TEST_UART_2_Read_Write(){
    BSP_UART_Init(UART_2);
    const char* testStr = "Enter your favorite number:\n\r";
    BSP_UART_Write(UART_2, (char*) testStr , strlen(testStr));
    
    char input[128];
    BSP_UART_Read(UART_2, input);

    BSP_UART_Write(UART_2, "\n", 1);
    BSP_UART_Write(UART_2, input, 4);
}

int main(void) {

    TEST_UART_2_Read_Write();
    

    while(1);
        // char out[2][TX_SIZE];
        // BSP_UART_Read(UART_2, out[UART_2]);
        // BSP_UART_Read(UART_3, out[UART_3]);
        // out[UART_2][strlen(out[UART_2])-1] = 0; // remove the newline, so we can print both in one line for now
        // out[UART_3][strlen(out[UART_3])-1] = 0;
        // printf("UART 2: %s\tUART 3: %s\r", out[UART_2], out[UART_3]);
        // /*
        //  * If a long message is sent before a short message, the messages will overlap
        //  * on the display. This is not an issue with UART_2, but just a consequence of
        //  * how these tests must be structured and outputted.
        //  */
}
