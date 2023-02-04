#include "Pedals.h"

int main(void){
    Pedals_Init();
    BSP_UART_Init(UART_2);
    while(1){
        printf("%d\n\r", Pedals_Read(ACCELERATOR));
        for(int i = 0; i < 9999999; i++){}
    }
}