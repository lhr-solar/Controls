/* Copyright (c) 2020 UT Longhorn Racing Solar */

#include "ReadCarCAN.h"
#include "Contactors.h"

#define THRESHOLD 3

void Task_ReadCarCAN(void *p_arg)
{
    car_state_t *car = (car_state_t *)p_arg;

    uint8_t buffer[8]; // buffer for CAN message

    static int faultCounter = 0;

    OS_ERR err;

    uint32_t canId;

    while (1)
    {
        // Check if BPS sent us a message
        if (CANbus_Read(&canId, buffer, CAN_BLOCKING) == SUCCESS)
        {
            // If charge_enable, set regen flag
            if (canId == CHARGE_ENABLE)
            {
                car->IsRegenBrakingAllowed = (buffer[0] == 0) ? OFF : ON;
                Contactors_Set(ARRAY, car->IsRegenBrakingAllowed);
                faultCounter = 0;
            }
            else
            {
                // If we didn't get a message, something might have gone wrong
                faultCounter++;
            }
        }

        if (faultCounter >= THRESHOLD)
        {
            car->IsRegenBrakingAllowed = OFF;
            Contactors_Set(ARRAY, OFF);
        }

        OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_NON_STRICT, &err);

        if (err != OS_ERR_NONE)
        {
            car->ErrorCode.ReadCANErr = ON;
        }
    }
}