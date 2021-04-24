/* Copyright (c) 2020 UT Longhorn Racing Solar */

#include "UpdateLights.h"
#include "Tasks.h"
#include "Lights.h"

#define BRAKE_THRESHOLD 2

void Task_BlinkLight(void *p_arg) {} // TODO remove this

void Task_UpdateLights(void *p_arg) {
    car_state_t *car = (car_state_t *) p_arg;

    OS_ERR err;

    Lights_Init();

    for (int i=A_CNCTR; i <= RSVD_LED; i++) {
        Lights_Set(i, OFF);
    }

    // BlinkLight handles turn signals and hazards
    INIT_TASK(BlinkLight, TASK_BLINK_LIGHT_PRIO, p_arg, err);

    while (1) {
        // Brake lights
        if (car->BrakePedalPercent >= 2) {
            Lights_Set(BrakeLight, ON);
        } else {
            Lights_Set(BrakeLight, OFF);
        }

        // headlights
        if (car->SwitchStates.HDLT == ON) {
            Lights_Set(Headlight_ON, ON);
        } else {
            Lights_Set(Headlight_ON, OFF);
        }

        // faults

        OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_NON_STRICT, &err);
    }
}