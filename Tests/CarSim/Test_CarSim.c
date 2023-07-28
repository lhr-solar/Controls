#include "BSP_CAN.h"
#include <stdio.h>

#include "Test_Runner.h"

// Handlers
static void RX_Handler(){
    uint32_t id;
    uint8_t data[8];
    
    BSP_CAN_Read(CAN_1, &id, data);
}

void Test_Setup(){
    BSP_CAN_Init(CAN_1, (callback_t)RX_Handler, NULL, NULL, 0);
}

void Test_Start(){
    while(1);
}