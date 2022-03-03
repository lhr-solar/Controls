#include "FaultState.h"
#include "Contactors.h"
#include "os.h"
#include "Tasks.h"

static void ArrayKill(void) {
    Contactors_Set(ARRAY, OFF);
}

static void MotorKill(void) {
    Contactors_Set(MOTOR, OFF);
}

static void ArrayMotorKill(void) {
    ArrayKill();
    MotorKill();
}

void EnterFaultState(void) {
    if(FaultBitmap.Fault_OS){
        ArrayMotorKill();
    }
    else if(FaultBitmap.Fault_TRITIUM){
        ArrayMotorKill();
    }
    else if(FaultBitmap.Fault_READBPS){
        ArrayMotorKill();
    }
    else if(FaultBitmap.Fault_UNREACH){
        ArrayMotorKill();
    }
    else if(FaultBitmap.Fault_DISPLAY){
        // TODO: Send reset command to display ("rest")
        // To be implemented when display driver is complete
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