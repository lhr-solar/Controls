#include "FaultState.h"
#include "Contactors.h"
#include "os.h"
#include "Tasks.h"

static void ArrayKill(void) {
    Contactors_Set(ARRAY_CONTACTOR, OFF);
}

static void MotorKill(void) {
    Contactors_Set(MOTOR_CONTACTOR, OFF);
}

static void ArrayMotorKill(void) {
    ArrayKill();
    MotorKill();
}

void EnterFaultState(void) {
    if(FaultBitmap & FAULT_OS){
        ArrayMotorKill();
    }
    else if(FaultBitmap & FAULT_TRITIUM){
        ArrayMotorKill();
    }
    else if(FaultBitmap & FAULT_READBPS){
        ArrayMotorKill();
    }
    else if(FaultBitmap & FAULT_UNREACH){
        ArrayMotorKill();
    }
    else if(FaultBitmap & FAULT_DISPLAY){
        // TODO: Send reset command to display ("rest")
        // To be implemented when display driver is complete
    }

    while(1){}
}

void Task_FaultState(void *p_arg) {
    OS_ERR err;
    CPU_TS ts;

    FaultBitmap = FAULT_NONE;
    OSErrLocBitmap = OS_NONE_LOC;
    TritiumErrorBitmap = T_NONE;

    // Block until fault is signaled by an assert
    OSSemPend(&FaultState_Sem4, 0, OS_OPT_PEND_BLOCKING, &ts, &err);

    EnterFaultState();
}