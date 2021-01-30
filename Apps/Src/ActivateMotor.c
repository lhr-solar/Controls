/* Copyright (c) 2020 UT Longhorn Racing Solar */

#include "ActivateMotor.h"

void Task_MotorConnection(void *p_arg) {
    (void) p_arg;

    OS_ERR err;
    CPU_TS  ts;

    OSSemPend(&ActivateMotor_Sem4, 0, OS_OPT_PEND_BLOCKING, &ts, &err);
    // TODO: check for errors

    Precharge_Write(MOTOR_PRECHARGE, ON); // Activate the motor recharge
    Contactors_Init(MOTOR);

    OSTimeDlyHMSM(0, 0, 5, 0, OS_OPT_TIME_HMSM_STRICT, &err);
    // TODO: check for errors

    Contactors_Set(MOTOR, ON); // Actually activate motor contactors

    OSSemPost(&SendTritium_Sem4, OS_OPT_POST_1, &err);
    // TODO: check for errors

    OSTaskDel(NULL, &err);
}