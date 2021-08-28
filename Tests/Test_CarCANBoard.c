/* Copyright (c) 2020 UT Longhorn Racing Solar */

#include "common.h"
#include "config.h"
#include <bsp.h>
#include <unistd.h>
#include "CANbus.h"

int main(void){
    BSP_UART_Init(UART_2);
    CANbus_Init();

    uint32_t ids[10] = {MC_BUS, VELOCITY, MC_PHASE_CURRENT, VOLTAGE_VEC, CURRENT_VEC, BACKEMF, TEMPERATURE,
                        CAR_STATE, MOTOR_DISABLE, CHARGE_ENABLE};

    CANData_t data;
    data.d = 0xFFFF8765;
    

    CANPayload_t payload;
    payload.data = data;
    payload.bytes = 4;

    int size;

    size = 0;
    char str[128];
    uint8_t output;


    output = CANbus_Send(ids[0], payload,CAN_BLOCKING);

    //for (volatile int i = 0; i < 1000000; i++);

    //data.d = 0xFFFFFFFF;
    //output = CANbus_Send(0xFFF, payload);
    /*for(int i=0; i<sizeof(ids)/sizeof(ids[0]); i++){
        output = CANbus_Send(ids[i], payload);
        if(output == 0) {
            i--;
            continue;
        }
        size += sprintf(str, "OUTPUT: %d ", output);
    }*/

    while(1);
}
