#include "os.h"
#include "Tasks.h"
#include "BSP_UART.h"
#include "CANbus.h"
#include "Contactors.h"
#include "Minions.h"
#include "Pedals.h"

char input[500] = {0};

int main(void){
    BSP_UART_Init(UART_2);

    // while(1){
    //     printf("prompt: ");
    //     // for(int i = 0; i < 99999999; i++){}
    //     if(BSP_UART_Read(UART_2, input)){
    //         printf("Here is what you typed: %s\n\r", input);
    //     }

    //     // while (USART_GetFlagStatus(UART_2, USART_FLAG_RXNE) == RESET);
    //     // printf("%c", USART_ReceiveData(UART_2));
    // }
    CANbus_Init(CAN_1);
    Contactors_Init();
    Minion_Init();
    Pedals_Init();
    Task_CommandLine(NULL);
}