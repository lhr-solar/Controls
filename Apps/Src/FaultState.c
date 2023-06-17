/** 
 * @copyright Copyright (c) 202X UT Longhorn Racing Solar
 * @file FaultState.c
 * @brief 
 * 
 * This contains 
 * 
 * @author 
*/

#include "FaultState.h"
#include "Display.h"
#include "Contactors.h"
#include "os.h"
#include "Tasks.h"
#include "Contactors.h"
#include "Minions.h"
#include "UpdateDisplay.h"
#include "ReadTritium.h"
extern const PinInfo_t PINS_LOOKARR[]; // For GPIO writes. Externed from Minions Driver C file.

/**
 * Semaphores
 */
OS_SEM FaultState_Sem4;
OS_SEM ExceptionProtection_Sem4;

// current exception is initialized
exception_t currException = {.prio=PRI_RECOV,.message=NULL,.callback=NULL};

/**
 * @brief   Assert Error if non OS function call fails
 * @param   exception non OS Error that occurred
 */
void assertExceptionError(exception_t exception){
    OS_ERR err;
    CPU_TS ticks;

    OSSemPend(&ExceptionProtection_Sem4, 0, OS_OPT_POST_NONE, &ticks, &err);
    assertOSError(OS_FAULT_STATE_LOC, err); 

    currException = exception;

    OSSemPost(&FaultState_Sem4, OS_OPT_POST_1, &err);
    //printf("\n\rThis should happen after the exception is handled");
    assertOSError(OS_FAULT_STATE_LOC, err);
}
        
static void nonrecoverableFaultHandler(){
    // Turn additional brakelight on to indicate critical error
    BSP_GPIO_Write_Pin(PINS_LOOKARR[BRAKELIGHT].port, PINS_LOOKARR[BRAKELIGHT].pinMask, true);

    // Array motor kill
    BSP_GPIO_Write_Pin(CONTACTORS_PORT, ARRAY_CONTACTOR_PIN, OFF);
    BSP_GPIO_Write_Pin(CONTACTORS_PORT, MOTOR_CONTACTOR_PIN, OFF);
    while(1){;} //nonrecoverable
}


void EnterFaultState(void) {

    printf("%s", currException.message);
    if(currException.callback != NULL){
        currException.callback();
    } // Custom callback

    switch (currException.prio)
    {
    case PRI_RESERVED:
        // Things could always be worse
        // (Reserved for future cases of greater severity)
    case PRI_NONRECOV:
        nonrecoverableFaultHandler();
        break;
    case PRI_RECOV:
        // Priority level 2 only calls callback
        break;
    default:
        break;
    }
    
}

void Task_FaultState(void *p_arg) {
    OS_ERR err;
    CPU_TS ts;

    OSSemCreate(&ExceptionProtection_Sem4, "Fault State Exception Protection Semaphore", 1, &err); // To ensure currException won't be replaced too soon
    OSSemCreate(&FaultState_Sem4, "Fault State Semaphore", 0, &err);

    // Block until fault is signaled by an assert
    while(1){
        OSSemPend(&FaultState_Sem4, 0, OS_OPT_PEND_BLOCKING, &ts, &err);
        EnterFaultState();
        OSSemPost(&ExceptionProtection_Sem4, OS_OPT_POST_NONE, &err);
        assertOSError(OS_FAULT_STATE_LOC, err);  // We've finished handling the error and can post the mutex now
    }
}