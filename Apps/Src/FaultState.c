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
 * Macros for initializing the exception
*/
#define NEW_EXCEPTION(exception_name) exception_t exception_name = {.prio=-1,.message=NULL,.callback=NULL}

/**
 * Semaphores
 */
OS_SEM FaultState_Sem4;

/**
 * Mutex
 */
OS_MUTEX FaultState_Mutex;

// current exception is initialized
NEW_EXCEPTION(currException);

void _assertError(exception_t exception){
    OS_ERR err;
    CPU_TS ticks;

    OSMutexPend(&FaultState_Mutex, 0, OS_OPT_POST_NONE, &ticks, &err);
    assertOSError(OS_SEND_CAN_LOC, err);  // TODO: need location?

    currException = exception;

    OSSemPost(&FaultState_Sem4, OS_OPT_POST_1, &err);
    assertOSError(OS_SEND_CAN_LOC, err);
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
    case 0:
        // Things could always be worst
    case 1:
        nonrecoverableFaultHandler();
        break;
    case 2:
        // Priority level 2 only calls callback
        break;
    default:
        break;
    }
    
}

void Task_FaultState(void *p_arg) {
    OS_ERR err;
    CPU_TS ts;

    OSMutexCreate(&FaultState_Mutex, "Fault state mutex", &err);
    OSSemCreate(&FaultState_Sem4, "Fault State Semaphore", 0, &err);

    // Block until fault is signaled by an assert
    while(1){
        OSSemPend(&FaultState_Sem4, 0, OS_OPT_PEND_BLOCKING, &ts, &err);
        //fromThread = true;
        EnterFaultState();
        OSMutexPost(&FaultState_Mutex, OS_OPT_POST_NONE, &err);
        assertOSError(OS_SEND_CAN_LOC, err);  // We've finished handling the error and can post the mutex now // TODO: need location?
        //fromThread = false;
        OSTimeDlyHMSM(0,0,0,5,OS_OPT_TIME_HMSM_STRICT,&err);
    }
}