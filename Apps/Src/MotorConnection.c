/* Copyright (c) 2020 UT Longhorn Racing Solar */

#include "MotorConnection.h"

void Task_SendCarCAN(void *p_arg) {} // TODO: remove this

static void motor_startup(OS_ERR *err) {
    Precharge_Write(MOTOR_PRECHARGE, ON); // Activate the motor recharge

    OSTimeDlyHMSM(0, 0, 5, 0, OS_OPT_TIME_HMSM_STRICT, &err);
    // TODO: check for errors

    Contactors_Set(MOTOR, ON); // Actually activate motor contactors
    Precharge_Write(MOTOR_PRECHARGE, OFF);
}


void Task_MotorConnection(void *p_arg) {
    car_state_t *car_state = (car_state_t *) p_arg;

    OS_ERR err;
    CPU_TS ts;

    Contactors_Init(MOTOR);
    motor_startup(&err);

    // Spawn SendCarCAN
    INIT_TASK(SendCarCAN, TASK_SEND_CAR_CAN_PRIO, p_arg, err);
    // TODO: check for task creation error

    // Spawn ReadTritium
    INIT_TASK(ReadTritium, TASK_READ_TRITIUM_PRIO, p_arg, err);
    // TODO: check for task creation error

    // Spawn SendTritium
    INIT_TASK(SendTritium, TASK_SEND_TRITIUM_PRIO, p_arg, err);
    // TODO: check for task creation error

    while (1) {
        // Wait until some change needs to be made to the motor state
        OSSemPend(&MotorConnectionChange_Sem4, 0, OS_OPT_PEND_BLOCKING, &ts, &err);

        State desiredState = car_state->ShouldMotorBeActivated;
        State currentState = Contactors_Get(MOTOR);

        if (desiredState == ON && currentState != ON) {
            motor_startup(&err); // Reactivate the array
            OSTaskSemPost(&SendCarCAN_TCB, OS_OPT_POST_NONE, &err);
            OSTaskSemPost(&ReadTritium_TCB, OS_OPT_POST_NONE, &err);
            OSTaskSemPost(&SendTritium_TCB, OS_OPT_POST_NONE, &err);
        } else if (desiredState != ON && currentState == ON) {
            Contactors_Set(MOTOR, OFF); // Deactivate the array
            OSTaskSemPost(&SendCarCAN_TCB, OS_OPT_POST_NONE, &err);
            OSTaskSemPost(&ReadTritium_TCB, OS_OPT_POST_NONE, &err);
            OSTaskSemPost(&SendTritium_TCB, OS_OPT_POST_NONE, &err);
        }

    }

}