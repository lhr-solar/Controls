#include "FaultState.h"
#include "Contactors.h"
#include "os.h"
#include "Tasks.h"
#include "MotorController.h"
#include "Contactors.h"


static void ArrayMotorKill(void) {
    BSP_GPIO_Write_Pin(CONTACTORS_PORT, ARRAY_CONTACTOR_PIN, OFF);
    BSP_GPIO_Write_Pin(CONTACTORS_PORT, ARRAY_CONTACTOR_PIN, OFF);
    BSP_GPIO_Write_Pin(CONTACTORS_PORT, ARRAY_CONTACTOR_PIN, OFF);
}

/**
 * @brief Enter a shutdown state when a non-recoverable fault has occurred
 * 
 */
void EnterShutdown(){
    while(1){};
};

void EnterFaultState(void) {
    if(FaultBitmap & FAULT_OS){
        ArrayMotorKill();
        EnterShutdown();
    }
    else if(FaultBitmap & FAULT_TRITIUM){ //This gets tripped by the ReadTritium thread
        tritium_error_code_t TritiumError = MotorController_getTritiumError(); //get error code to segregate based on fault type
        ArrayMotorKill();
        EnterShutdown();
    }
    else if(FaultBitmap & FAULT_READBPS){ //This has been put in with future development in mind, it is not currently tripped by anything.
        ArrayMotorKill();
        EnterShutdown();
    }
    else if(FaultBitmap & FAULT_UNREACH){ //unreachable code
        ArrayMotorKill();
        EnterShutdown();
    }
    else if(FaultBitmap & FAULT_DISPLAY){
        // TODO: Send reset command to display ("reset")
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
    while(1){
        OSSemPend(&FaultState_Sem4, 0, OS_OPT_PEND_BLOCKING, &ts, &err);
        EnterFaultState();
    }
}