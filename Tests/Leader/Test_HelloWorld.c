#include "common.h"
#include "config.h"
#include <bsp.h>

int main(void){
    BSP_UART_Init(UART_2);
    
    while(1){
        printf("Hello World\n");
        for(int i=0; i<5000; i++){}
    }

    return 0;
}