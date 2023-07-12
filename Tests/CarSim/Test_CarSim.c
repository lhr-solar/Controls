#include "BSP_CAN.h"
#include <stdio.h>

#include "Test_Resource.h"

// Handlers
static void RX_Handler(){
    uint32_t id;
    uint8_t data[8];
    
    BSP_CAN_Read(CAN_1, &id, data);

    printf("RX ID: 0x%03x\n");
    printf("RX Data: 0x%08x\n");
}

static void Test_Setup(){
    BSP_CAN_Init(CAN_1, (callback_t)RX_Handler, NULL, NULL, 0);
}

static void Test_Start(void *arg){
    while(1){

    }
}