/**
 * @file
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @brief Test file for library to interact with UART
 *
 * Run this test in conjunction with the simulator
 * GUI. As this generates randomized values, the display
 * will update the values accordingly to show that the
 * display is reading the UART properly
 */

#include <bsp.h>

#include "common.h"
#include "config.h"

#define TX_SIZE 128

int main(void) {
  // BSP_UART_Init(UART_2);
  BSP_UART_Init(UART_3);
  const char* testStr = "\xff\xff\xffpage 2\xff\xff\xff";
  BSP_UART_Write(UART_3, (char*)testStr, strlen(testStr));
  while (1) {
    volatile int x = 0;
    x++;
  }

  // char out[2][TX_SIZE];
  // BSP_UART_Read(UART_2, out[UART_2]);
  // BSP_UART_Read(UART_3, out[UART_3]);
  // out[UART_2][strlen(out[UART_2])-1] = 0; // remove the newline, so we can
  // print both in one line for now out[UART_3][strlen(out[UART_3])-1] = 0;
  // printf("UART 2: %s\tUART 3: %s\r", out[UART_2], out[UART_3]);
  // /*
  //  * If a long message is sent before a short message, the messages will
  //  overlap
  //  * on the display. This is not an issue with UART_2, but just a consequence
  //  of
  //  * how these tests must be structured and outputted.
  //  */
}
