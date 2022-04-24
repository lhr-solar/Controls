#include "FaultState.h"
#include "Contactors.h"
#include "os.h"
#include "Tasks.h"

static void ArrayKill(void) {
    Contactors_Disable(ARRAY_CONTACTOR);
}

static void MotorKill(void) {
    Contactors_Disable(MOTOR_CONTACTOR);
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

    // Block until fault is signaled by an assert
    OSSemPend(&FaultState_Sem4, 0, OS_OPT_PEND_BLOCKING, &ts, &err);

    EnterFaultState();
}