#include "FaultState.h"
#include "Contactors.h"

static void ArrayKill(void) {
    Contactors_Set(ARRAY, OFF);
}

static void MotorKill(void) {
    Contactors_Set(MOTOR, OFF);
}

void EnterFaultState(void) {
    if(FaultBitmap.Fault_OS){
        ArrayKill();
        MotorKill();
    }
    else if(FaultBitmap.Fault_TRITIUM){
        ArrayKill();
        MotorKill();
    }
    else if(FaultBitmap.Fault_READBPS){
        ArrayKill();
        MotorKill();
    }
    else if(FaultBitmap.Fault_UNREACH){
        ArrayKill();
        MotorKill();
    }
    else if(FaultBitmap.Fault_DISPLAY){
        // TODO: What should be done during a display fault?
    }

    while(1){}
}

void Task_FaultState(void *p_arg) {
    (void) p_arg;

    OS_ERR err;
    CPU_TS ts;

    FaultBitmap.bitmap = 0; // Initialize to no faults

    // Block until fault is signaled by an assert
    OSSemPend(&FaultState_Sem4, 0, OS_OPT_PEND_BLOCKING, &ts, &err);

    EnterFaultState();
}