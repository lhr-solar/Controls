#include <stdio.h>
#include "Minions.h"
int main(void){
    Minion_Init();
    BSP_UART_Init(UART_2);
    Minion_Error_t e;

    while(1){
        printf("IGN_1: %d\n\r", Minion_Read_Pin(IGN_1, &e));
        printf("IGN_2: %d\n\r", Minion_Read_Pin(IGN_2, &e));

        for(int i =0; i < 999999; i++){}
    }
}