/* Copyright (c) 2020 UT Longhorn Racing Solar */
#include "FaultState.h"

/*
 * Note: do not call this directly if it can be helped.
 * Instead, call an RTOS function to unblock the mutex
 * that the Fault Task is pending on.
 */
void EnterFaultState(void *p_arg) {
    car_state_t *car_state = (car_state_t *) p_arg;

    fault_bitmap_t FaultBitmap = car_state->FaultBitmap;

    if(FaultBitmap.Fault_OS){
        //EEPROM_LogError(FaultBitmap);
        arrayKill();
        motor_kill();
    }
    else if(FaultBitmap.Fault_TRITIUM){
        //EEPROM_LogError(FaultBitmap);
        arrayKill();
        motor_kill();
    }
    else if(FaultBitmap.Fault_READBPS){
        //EEPROM_LogError(FaultBitmap);
        arrayKill();
        motor_kill();
    }
    else if(FaultBitmap.Fault_UNREACH){
        //EEPROM_LogError(FaultBitmap);
        arrayKill();
        motor_kill();
    }
    else if(FaultBitmap.Fault_DISPLAY){
        //EEPROM_LogError(FaultBitmap);
    }

    while(1){}
}


void Task_FaultState(void *p_arg) {
    car_state_t *car_state = (car_state_t *) p_arg;
    OS_ERR err;
    CPU_TS ts;
    
    // BLOCKING =====================
    // Wait until a FAULT is signaled by another task.
    OSSemPend(&Fault_State_Sem4, 0, OS_OPT_PEND_BLOCKING, &ts, &err);
    
    EnterFaultState(car_state);
}