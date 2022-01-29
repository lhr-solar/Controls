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

            if(err != OS_ERR_NONE){
                car->ErrorCode.ReadCANErr = ON;
            }
        } else if (err != OS_ERR_PEND_WOULD_BLOCK) {
            car->ErrorCode.ReadCANErr = ON;
        }

        // Normal task countinues here

        uint32_t canId;

        // Check if BPS sent us a message
        if (CANbus_Read(&canId, buffer, CAN_NON_BLOCKING) == SUCCESS) {
            // If charge_enable, set regen flag
            if (canId == CHARGE_ENABLE) {
                car->IsRegenBrakingAllowed = (buffer[0] == 0) ? OFF : ON;
                faultCounter = 0;
            } else {
                // If we didn't get a message, something might have gone wrong
                faultCounter++;
            }
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