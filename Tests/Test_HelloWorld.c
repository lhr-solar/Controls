#include "common.h"
#include "config.h"
#include <bsp.h>

int main(void){
    BSP_UART_Init(UART_2);
    BSP_GPIO_Init(PORTA, 0, INPUT);
    
    while(1){
        printf("GPIO: %d\n\r", BSP_GPIO_Read(PORTA));
        for(int i=0; i<5000; i++){}
    }

    return 0;
}