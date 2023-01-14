#include "common.h"
#include "config.h"
#include <bsp.h>

int main(void){
    BSP_UART_Init(UART_2);
    BSP_UART_Write(UART_2, "Hello World!\r", 14);
    return 0;
}