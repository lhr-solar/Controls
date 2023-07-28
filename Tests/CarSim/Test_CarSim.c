#include "BSP_CAN.h"
#include <stdio.h>

#define TEST_USE_RTOS 0
#include "Test_Runner.h"

// Handlers
static void RX_Handler(){
    uint32_t id;
    uint8_t data[8];
    
    BSP_CAN_Read(CAN_3, &id, data);
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