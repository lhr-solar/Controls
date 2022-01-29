/* Copyright (c) 2020 UT Longhorn Racing Solar */

#include "ReadCarCAN.h"

#define THRESHOLD 3

void Task_ReadCarCAN(void *p_arg) {
    car_state_t *car = (car_state_t *) p_arg;

    uint8_t buffer[8]; // buffer for CAN message

    static int faultCounter = 0;

    OS_ERR err;
    CPU_TS ts;

    while (1) {
        // Check if BPS sent us a message
        if (CANbus_Read(buffer) == SUCCESS) {
            long msg = *((long *)(&buffer[0]));
            if (msg == 0) {
                car->IsRegenBrakingAllowed = OFF;
            } else {
                car->IsRegenBrakingAllowed = ON;
            }

            faultCounter = 0;
        } else {
            faultCounter++;
        }

        if (faultCounter >= THRESHOLD) {
            car->IsRegenBrakingAllowed = OFF;
        }

        OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_NON_STRICT, &err);
        
        if(err != OS_ERR_NONE){
            car->ErrorCode.ReadCANErr = ON;
        }
    }
}