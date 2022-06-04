#include "FaultState.h"
#include "Contactors.h"
#include "os.h"
#include "Tasks.h"


/**
 * @brief Non-OS dependent way to kill all contactors
 */

static void ArrayMotorKill(void) {
    BSP_GPIO_Write_Pin(CONTACTORS_PORT, ARRAY_CONTACTOR_PIN, OFF);
    BSP_GPIO_Write_Pin(CONTACTORS_PORT, ARRAY_CONTACTOR_PIN, OFF);
    BSP_GPIO_Write_Pin(CONTACTORS_PORT, ARRAY_CONTACTOR_PIN, OFF);
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

/**
 * @brief Fault task, highest priority thread that shuts down the car
 * 
 * @param p_arg 
 */
void Task_FaultState(void *p_arg) {
    OS_ERR err = OS_ERR_NONE;
    CPU_TS ts;

    FaultBitmap = FAULT_NONE;
    OSErrLocBitmap = OS_NONE_LOC;

    // Block until fault is signaled by an assert
    OSSemPend(&FaultState_Sem4, 0, OS_OPT_PEND_BLOCKING, &ts, &err);

    EnterFaultState();
}