#include "common.h"
#include "config.h"
#include <bsp.h>

int main(void){
    BSP_UART_Init(UART_2);
    while(1){
        BSP_UART_Write(UART_2, "Hello World!\n", 14);
        for(int i = 0; i < 999; i++){}
    }
    return 0;
}