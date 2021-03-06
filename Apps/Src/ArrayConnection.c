/* Copyright (c) 2020 UT Longhorn Racing Solar */

#include "ArrayConnection.h"
#include "Contactors.h"

static void arrayStartup(OS_ERR *err) {
    Precharge_Write(ARRAY_PRECHARGE, ON); // Turn on the array precharge

    OSTimeDlyHMSM(0, 0, PRECHARGE_ARRAY_DELAY, 0, OS_OPT_TIME_HMSM_NON_STRICT, &err);
    // TODO: error handling

    Contactors_Set(ARRAY, ON); // Actually activate the contactor
    Precharge_Write(ARRAY_PRECHARGE, OFF); // Deactivate the array precharge
}

void Task_ArrayConnection(void *p_arg) {

    car_state_t *car_state = (car_state_t *) p_arg;

    OS_ERR err;
    CPU_TS ts;

    Contactors_Init(ARRAY); //  Initialize the contactors
    arrayStartup(&err);

    // Create ReadCarCAN
    OSTaskCreate(
        (OS_TCB*)&ReadCarCAN_TCB,
        (CPU_CHAR*)"ReadCarCAN",
        (OS_TASK_PTR)Task_ReadCarCAN,
        (void*) car_state,
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
    // TODO: error handling

    while (1) {
        // Wait until some change needs to be made to the array state
        OSSemPend(&ArrayConnectionChange_Sem4, 0, OS_OPT_PEND_BLOCKING, &ts, &err);

        State desiredState = car_state->ShouldArrayBeActivated;
        State currentState = Contactors_Get(ARRAY);


        if (desiredState == ON && currentState != ON) {
            arrayStartup(&err); // Reactivate the array
            OSTaskSemPost(&ReadCarCAN_TCB, OS_OPT_POST_NONE, &err);
        } else if (desiredState != ON && currentState == ON) {
            Contactors_Set(ARRAY, OFF); // Deactivate the array
            OSTaskSemPost(&ReadCarCAN_TCB, OS_OPT_POST_NONE, &err);
        }
        
    }
}