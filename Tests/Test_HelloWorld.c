#include "common.h"
#include "config.h"
#include <bsp.h>

int main(void){
    BSP_UART_Init(UART_2);
    printf("Hello World\n");

    while(1){}

    return 0;
}