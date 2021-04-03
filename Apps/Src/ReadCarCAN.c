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
        OSTaskSemPend(0, OS_OPT_PEND_NON_BLOCKING, &ts, &err);
        if (err == OS_ERR_NONE) {
            // A signal was received, so the task should wait until signaled again
            OSTaskSemPend(0, OS_OPT_PEND_BLOCKING, &ts, &err);
            // TODO: error handling
        } else if (err != OS_ERR_PEND_WOULD_BLOCK) {
            // TODO: error handling
        }

        // Normal task countinues here

        // Check if BPS sent us a message
        if (CANbus_Read(buffer) == SUCCESS) {
            if (buffer[0] == 0) {
                car->IsRegenBrakingAllowed = OFF;
            } else {
                car->IsRegenBrakingAllowed = ON;
            }

            faultCounter = 0;
        } else {
            faultCounter++;
        }

        if (faultCounter >= THRESHOLD) {
            car->IsRegenBrakingAllowed = ON;
        }

        OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_NON_STRICT, &err);
        // TODO: error handling
        
    }
}