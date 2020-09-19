#include "common.h"
#include "BSP_UART.h"

int main() {
    BSP_UART_Init(UART_3);

    while(1) {
        printf("This is a test.\n");
        for(int i = 0; i < 999999; ++i);
    }

}