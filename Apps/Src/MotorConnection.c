/* Copyright (c) 2020 UT Longhorn Racing Solar */

#include "MotorConnection.h"

static void motor_startup(OS_ERR *err) {
    // Precharge_Write(MOTOR_PRECHARGE, ON); // Activate the motor recharge

    OSTimeDlyHMSM(0, 0, 5, 0, OS_OPT_TIME_HMSM_STRICT, err);
    // TODO: check for errors

    Contactors_Set(MOTOR_CONTACTOR, ON); // Actually activate motor contactors
    // Precharge_Write(MOTOR_PRECHARGE, OFF);
}


void Task_MotorConnection(void *p_arg) {
    car_state_t *car_state = (car_state_t *) p_arg;

    OS_ERR err;
    CPU_TS ts;

    Contactors_Init(MOTOR_CONTACTOR);
    motor_startup(&err);
    if(err != OS_ERR_NONE){
        car_state->ErrorCode.MotorConnectionErr = ON;
    }

    // Spawn SendCarCAN
    OSTaskCreate(
        (OS_TCB*)&SendCarCAN_TCB,
        (CPU_CHAR*)"SendCarCAN",
        (OS_TASK_PTR)Task_SendCarCAN,
        (void*) car_state,
        (OS_PRIO)TASK_SEND_CAR_CAN_PRIO,
        (CPU_STK*)SendCarCAN_Stk,
        (CPU_STK_SIZE)WATERMARK_STACK_LIMIT,
        (CPU_STK_SIZE)TASK_SEND_CAR_CAN_STACK_SIZE,
        (OS_MSG_QTY) 0,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR|OS_OPT_TASK_STK_CHK),
        (OS_ERR*)&err
    );
    // TODO: check for task creation error
    if(err != OS_ERR_NONE){
        car_state->ErrorCode.SendCANErr = ON;
    }

    // Spawn ReadTritium
    OSTaskCreate(
        (OS_TCB*)&ReadTritium_TCB,
        (CPU_CHAR*)"ReadTritium",
        (OS_TASK_PTR)Task_ReadTritium,
        (void*) car_state,
        (OS_PRIO)TASK_READ_TRITIUM_PRIO,
        (CPU_STK*)ReadTritium_Stk,
        (CPU_STK_SIZE)WATERMARK_STACK_LIMIT,
        (CPU_STK_SIZE)TASK_READ_TRITIUM_STACK_SIZE,
        (OS_MSG_QTY) 0,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR|OS_OPT_TASK_STK_CHK),
        (OS_ERR*)&err
    );
    // TODO: check for task creation error
    if(err != OS_ERR_NONE){
        car_state->ErrorCode.ReadTritiumErr = ON;
    }

    // Spawn SendTritium
    OSTaskCreate(
        (OS_TCB*)&SendTritium_TCB,
        (CPU_CHAR*)"SendTritium",
        (OS_TASK_PTR)Task_SendTritium,
        (void*) car_state,
        (OS_PRIO)TASK_SEND_TRITIUM_PRIO,
        (CPU_STK*)SendTritium_Stk,
        (CPU_STK_SIZE)WATERMARK_STACK_LIMIT,
        (CPU_STK_SIZE)TASK_SEND_TRITIUM_STACK_SIZE,
        (OS_MSG_QTY) 0,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR|OS_OPT_TASK_STK_CHK),
        (OS_ERR*)&err
    );
    // TODO: check for task creation error
    if(err != OS_ERR_NONE){
        car_state->ErrorCode.SendTritiumErr = ON;
    }

    while (1) {
        // Wait until some change needs to be made to the motor state
        OSSemPend(&MotorConnectionChange_Sem4, 0, OS_OPT_PEND_BLOCKING, &ts, &err);

        State desiredState = car_state->ShouldMotorBeActivated;
        State currentState = Contactors_Get(MOTOR_CONTACTOR);

        if (desiredState == ON && currentState != ON) {
            motor_startup(&err); // Reactivate the array
            OSTaskResume(&SendCarCAN_TCB, &err);
            if(err != OS_ERR_NONE){
                car_state->ErrorCode.SendCANErr = ON;
            }
            OSTaskResume(&ReadTritium_TCB, &err);
            if(err != OS_ERR_NONE){
                car_state->ErrorCode.ReadTritiumErr = ON;
            }
            OSTaskResume(&SendTritium_TCB, &err);
            if(err != OS_ERR_NONE){
                car_state->ErrorCode.SendTritiumErr = ON;
            }
        } else if (desiredState != ON && currentState == ON) {
            Contactors_Set(MOTOR_CONTACTOR, OFF); // Deactivate the array
            OSTaskSuspend(&SendCarCAN_TCB, &err);
            if(err != OS_ERR_NONE){
                car_state->ErrorCode.SendCANErr = ON;
            }
            OSTaskSuspend(&ReadTritium_TCB, &err);
            if(err != OS_ERR_NONE){
                car_state->ErrorCode.ReadTritiumErr = ON;
            }
            OSTaskSuspend(&SendTritium_TCB, &err);
            if(err != OS_ERR_NONE){
                car_state->ErrorCode.SendTritiumErr = ON;
            }
        }

    }

}