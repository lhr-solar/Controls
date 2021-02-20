/* Copyright (c) 2020 UT Longhorn Racing Solar */

#include "ActivateMotor.h"


// TODO: remove dummy function definitions!
void Task_SendCarCAN (void *p_arg) {}
void Task_ReadCarCAN (void *p_arg) {}
void Task_ReadTritium(void *p_arg) {}
void Task_SendTritium(void *p_arg) {}


void Task_MotorConnection(void *p_arg) {
    (void) p_arg;

    OS_ERR err;

    Precharge_Write(MOTOR_PRECHARGE, ON); // Activate the motor recharge
    Contactors_Init(MOTOR);

    OSTimeDlyHMSM(0, 0, 5, 0, OS_OPT_TIME_HMSM_STRICT, &err);
    // TODO: check for errors

    Contactors_Set(MOTOR, ON); // Actually activate motor contactors

    // Now we need to spawn the four CAN related tasks

    // Spawn SendCarCAN
    OSTaskCreate(
        (OS_TCB*)&SendCarCAN_TCB,
        (CPU_CHAR*)"SendCarCAN",
        (OS_TASK_PTR)Task_SendCarCAN,
        (void*)NULL,
        (OS_PRIO)TASK_SEND_CAR_CAN_PRIO,
        (CPU_STK*)SendCarCAN_Stk,
        (CPU_STK_SIZE)WATERMARK_STACK_LIMIT,
        (CPU_STK_SIZE)TASK_SEND_CAR_CAN_STACK_SIZE,
        (OS_MSG_QTY)NULL,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    );
    // TODO: check for task creation error

    // Spawn ReadCarCAN
    OSTaskCreate(
        (OS_TCB*)&ReadCarCAN_TCB,
        (CPU_CHAR*)"ReadCarCAN",
        (OS_TASK_PTR)Task_ReadCarCAN,
        (void*)NULL,
        (OS_PRIO)TASK_READ_CAR_CAN_PRIO,
        (CPU_STK*)ReadCarCAN_Stk,
        (CPU_STK_SIZE)WATERMARK_STACK_LIMIT,
        (CPU_STK_SIZE)TASK_READ_CAR_CAN_STACK_SIZE,
        (OS_MSG_QTY)NULL,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    );
    // TODO: check for task creation error

    // Spawn ReadTritium
    OSTaskCreate(
        (OS_TCB*)&ReadTritium_TCB,
        (CPU_CHAR*)"ReadTritium",
        (OS_TASK_PTR)Task_ReadTritium,
        (void*)NULL,
        (OS_PRIO)TASK_READ_TRITIUM_PRIO,
        (CPU_STK*)ReadTritium_Stk,
        (CPU_STK_SIZE)WATERMARK_STACK_LIMIT,
        (CPU_STK_SIZE)TASK_READ_TRITIUM_STACK_SIZE,
        (OS_MSG_QTY)NULL,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    );

    // TODO: check for task creation error

    // Spawn SendTritium
    OSTaskCreate(
        (OS_TCB*)&SendTritium_TCB,
        (CPU_CHAR*)"SendTritium",
        (OS_TASK_PTR)Task_SendTritium,
        (void*)NULL,
        (OS_PRIO)TASK_SEND_TRITIUM_PRIO,
        (CPU_STK*)SendTritium_Stk,
        (CPU_STK_SIZE)WATERMARK_STACK_LIMIT,
        (CPU_STK_SIZE)TASK_SEND_TRITIUM_STACK_SIZE,
        (OS_MSG_QTY)NULL,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    );
    // TODO: check for task creation error

    OSTaskDel(NULL, &err); // we're done 
}