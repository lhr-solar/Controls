/* Copyright (c) 2020 UT Longhorn Racing Solar */

#include "ActivateArray.h"

void Task_ArrayConnection(void *p_arg) {
    OS_ERR err;
    CPU_TS  ts;

    // Wait until driver turned the key to the right place
    OSSemPend(&ActivateArray_Sem4, 0, OS_OPT_PEND_BLOCKING, &ts, &err);
    // TODO: error handling

    Precharge_Write(ARRAY_PRECHARGE, ON); // Turn on the array precharge
    Contactors_Init(ARRAY); //  Initialize the contactors

    OSTimeDlyHMSM(0, 0, 5, 0, (OS_OPT_TIME_HMSM_STRICT | OS_OPT_TIME_DLY), &err);
    // TODO: error handling

    Contactors_Set(ARRAY, ON); // Actually activate the contactor

    OSTaskDel(NULL, &err); // We're done
}