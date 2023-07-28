#include "BSP_CAN.h"
#include <stdio.h>

// Handlers
static void RX_Handler(){
    uint32_t id;
    uint64_t data;
    
    BSP_CAN_Read(CAN_1, &id, (uint8_t*)(&data));

    printf("RX ID: 0x%03lx\n\r", id);
    printf("RX Data: 0x%08llx\n\r", data);
}

int main(){
    // Disable interrupts
    __disable_irq();
    
    // Initialize drivers
    BSP_CAN_Init(CAN_3, (callback_t)RX_Handler, NULL, NULL, 0);
    BSP_UART_Init(UART_2);
    
    // Enable interrupts
    __enable_irq();
    while(1);
}