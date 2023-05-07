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

    output = CANbus_Send(ids[0], payload);

    while(1);
}
