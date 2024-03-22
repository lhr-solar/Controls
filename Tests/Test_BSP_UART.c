/**
 * Test file for library to interact with UART
 * 
 * 
 */

#include "common.h"
#include "config.h"
#include <bsp.h>

#define TX_SIZE 128

/**
 * Read a line using UART
 * Print the first 4 chars using UART
 */
void Test_UART_2_Read_Write(){
    const char* testStr = "Enter your favorite number:\n\r";
    BSP_UART_Write(UART_2, (char*) testStr , strlen(testStr));
    
    //128 is the minimum buffer size for BSP_UART_Read
    char input[128];
    BSP_UART_Read(UART_2, input);

    //Print the first 4 characters of the input
    BSP_UART_Write(UART_2, "\n\r", 2);
    BSP_UART_Write(UART_2, input, 4);
}

/**
 * Read a number using UART through scanf
 * Print that number using UART printf
*/
void Test_UART_2_Printf_Scanf(){
    int num;
    printf("Enter a Number and I'll add one to it!\n\r");
    scanf("%d", &num);
    printf("\n\r%d", num + 1);
}

/**
 * Write a 32 bit number to UART 3 and read it back
 * on a logic analyzer
*/
void Test_UART_3_Write(){
    const uint32_t test = 0xdeadbeef;
    BSP_UART_Write(UART_3, (char*) &test , 4);
}

int main(void) {
    BSP_UART_Init(UART_2);
    BSP_UART_Init(UART_3);

    Test_UART_2_Read_Write();
    Test_UART_2_Printf_Scanf();
    Test_UART_3_Write();

    while(1);

}
