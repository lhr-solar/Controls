/* Copyright (c) 2020 UT Longhorn Racing Solar */

#include "ArrayConnection.h"
#include "Contactors.h"


static void arrayStartup(car_state_t *car_state, OS_ERR *err) {
    Precharge_Write(ARRAY_PRECHARGE, ON); // Turn on the array precharge

    OSTimeDlyHMSM(0, 0, PRECHARGE_ARRAY_DELAY, 0, OS_OPT_TIME_HMSM_NON_STRICT, err);
    assertOSError(car_state, OS_ARRAY_LOC, *err);

    Contactors_Set(ARRAY, ON); // Actually activate the contactor
    Precharge_Write(ARRAY_PRECHARGE, OFF); // Deactivate the array precharge
}

void arrayKill(){
    Contactors_Set(ARRAY, OFF);
}

void Task_ArrayConnection(void *p_arg) {
    car_state_t *car_state = (car_state_t *) p_arg;

    OS_ERR err;
    CPU_TS ts;

    Contactors_Init(ARRAY); //  Initialize the contactors
    arrayStartup(car_state, &err);
    assertOSError(car_state, OS_ARRAY_LOC, err);

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
        (OS_MSG_QTY) 0,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    );
    
    assertOSError(car_state, OS_READ_CAN_LOC, err);

    while (1) {
        // Wait until some change needs to be made to the array state
        OSSemPend(&ArrayConnectionChange_Sem4, 0, OS_OPT_PEND_BLOCKING, &ts, &err);
        assertOSError(car_state, OS_ARRAY_LOC, err);

        State desiredState = car_state->ShouldArrayBeActivated;
        State currentState = Contactors_Get(ARRAY);

        if (desiredState == ON && currentState != ON) {
            arrayStartup(car_state, &err); // Reactivate the array
            OSTaskSemPost(&ReadCarCAN_TCB, OS_OPT_POST_NONE, &err);
            assertOSError(car_state, OS_ARRAY_LOC, err);
        } else if (desiredState != ON && currentState == ON) {
            Contactors_Set(ARRAY, OFF); // Deactivate the array
            OSTaskSemPost(&ReadCarCAN_TCB, OS_OPT_POST_NONE, &err);
            assertOSError(car_state, OS_ARRAY_LOC, err);
        }
        
        assertOSError(car_state, OS_ARRAY_LOC, err);
    }
}