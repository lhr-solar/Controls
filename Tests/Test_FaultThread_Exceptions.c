/**
 * Test file for fault state exceptions
 * 
 * This file tests fault state 
 * to ensure thrown exceptions are handled correctly
 * 
 * CHANGE THISXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 * <Run this test in conjunction with the simulator 
 * GUI. The user is prompted to specify the
 * contactor name and the state it should 
 * be set to. The input should be all caps and formatted
 * as such: CONTACTOR_NAME(MOTOR/ARRAY) STATE(ON/OFF)
 * Example inputs: "MOTOR ON", "ARRAY OFF"
 * The corresponding contactor will change state
 * in the GUI.>
 * 
 * 
 * @file 
 * @author Nathaniel Delgado (nathaniel.delgado@utexas.edu)
 * @brief Tests the fault state exception mechanism
 * @version idk
 * @date 2023-6-08
 *
 * @copyright Copyright (c) 2022 Longhorn Racing Solar
 *
 */

#include "BSP_UART.h"
#include "FaultState.h"
#include "Tasks.h"



/*** Task components ***/
static OS_TCB ManagerTaskTCB;
static OS_TCB ExceptionTaskTCB;
static CPU_STK ManagerTaskStk;
static CPU_STK ExceptionTaskStk;

// To do:
// Change all task initializations to be consistent in constant usage if both options work

// Initializes FaultState
void createFaultState(void) {
    OS_ERR err;
    
    OSTaskCreate( // Create fault task
        (OS_TCB*)&FaultState_TCB,
        (CPU_CHAR*)"Fault State",
        (OS_TASK_PTR)&Task_FaultState,
        (void*)NULL,
        (OS_PRIO)2,
        (CPU_STK*)FaultState_Stk,
        (CPU_STK_SIZE)128/10,
        (CPU_STK_SIZE)128,
        (OS_MSG_QTY)0,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    );
    assertOSError(OS_MAIN_LOC,err); 

}

// Initializes ReadTritium
void createReadTritium(void) {
    OS_ERR err;
 
    OSTaskCreate(
        (OS_TCB*)&ReadTritium_TCB,
        (CPU_CHAR*)"ReadTritium",
        (OS_TASK_PTR)Task_ReadTritium,
        (void*)NULL,
        (OS_PRIO)3,
        (CPU_STK*)ReadTritium_Stk,
        (CPU_STK_SIZE)WATERMARK_STACK_LIMIT,
        (CPU_STK_SIZE)TASK_READ_TRITIUM_STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)0,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    );
    assertOSError(OS_MAIN_LOC, err);
}

// Initializes ReadCarCAN
void createReadCarCAN(void) {
    OS_ERR err;

    OSTaskCreate( // Create readCarCAN task
        (OS_TCB*)&ReadCarCAN_TCB,
        (CPU_CHAR*)"Read Car CAN",
        (OS_TASK_PTR)Task_ReadCarCAN,
        (void*)NULL,
        (OS_PRIO)5,
        (CPU_STK*)ReadCarCAN_Stk,
        (CPU_STK_SIZE)128/10,
        (CPU_STK_SIZE)128,
        (OS_MSG_QTY)0,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    );
    assertOSError(OS_MAIN_LOC,err);

}

// Initializes UpdateDisplay
void createUpdateDisplay(void) {
    OS_ERR err;

    OSTaskCreate(
        (OS_TCB *)&UpdateDisplay_TCB,
        (CPU_CHAR *)"UpdateDisplay_TCB",
        (OS_TASK_PTR)Task_UpdateDisplay,
        (void *)NULL,
        (OS_PRIO)6,
        (CPU_STK *)UpdateDisplay_Stk,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE / 10,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)NULL,
        (void *)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR *)&err
    );
    assertOSError(OS_MAIN_LOC,err); 

}

// Creates a task to test exceptions on their own
// callback_function: pass in the callback function to use or NULL for no callbacks
void createExceptionTask(void * callback_function) {
    OS_ERR err;
     
     OSTaskCreate(
        (OS_TCB *)&ExceptionTaskTCB,
        (CPU_CHAR *)"ExceptionTask",
        (OS_TASK_PTR)ExceptionTaskStk,
        (void *)callback_function,
        (OS_PRIO)7,
        (CPU_STK *)ExceptionTaskStk,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE / 10,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)NULL,
        (void *)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR *)&err
    );
    assertOSError(OS_MAIN_LOC,err); 

}

// Callback function for ExceptionTask test exceptions
void exceptionCallback(void) {
    printf("\n\rException callback successfully executed.");
}


// Creates an exception of priority 1 and 2 to test
// test_callbacks: the callback function to use for the exceptions
void ExceptionTask(void* test_callbacks) {


    if (test_callbacks == NULL) {
        printf("\n\r Testing exceptions without callback functions");
    } else {
        printf("\n\r Testing exceptions with callback functions");
    }
    // Throw a priority 2 exception
    exception_t prio2Exception = {.prio = 2, .message = "\n\rprio2 exception message", .callback = test_callbacks};
    assertExceptionError(prio2Exception);
    printf("\n\rPrio 2 test exception with callback has been thrown");
    
    // Throw a priority 1 exception
    exception_t prio1Exception = {.prio = 1, .message = "\n\rprio1 exception message", .callback = test_callbacks};
    assertExceptionError(prio1Exception);
    printf("\n\rTest failed: Prio 1 exception did not immediately result in an unrecoverable fault");

}

// Test the assertOSError function by pending on a mutex that wasn't created
void OSErrorTask(void* arg) {
    OS_ERR err;
    OS_MUTEX testMut;
    CPU_TS ts;
    OSMutexPend(&testMut, 0, OS_OPT_PEND_NON_BLOCKING, &ts, &err);
    assertOSError(OS_MAIN_LOC, err);
    printf("assertOSError test failed: assertion did not immediately result in an unrecoverable fault");
}   


// A high-priority task that manages other tasks and runs the tests
void ManagerTask(void* arg) {
    CPU_Init();
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U) OSCfg_TickRate_Hz);
    OS_ERR err;

    // Test the exceptions mechanism by creating and throwing exceptions of priorities 1 and 2
    // Both with and without callback functions
    printf("=========== Testing exception priorities 1 and 2 ===========");
   
    createExceptionTask(NULL); // Test level 1 & 2 exceptions without callbacks
    assertOSError(OS_MAIN_LOC, err);
    OSTaskDel(&ExceptionTaskTCB, &err);
    assertOSError(OS_MAIN_LOC, err);

    createExceptionTask(exceptionCallback); // Test level 1 & 2 exceptions with callbacks
    assertOSError(OS_MAIN_LOC, err);
    OSTaskDel(&ExceptionTaskTCB, &err);
    assertOSError(OS_MAIN_LOC, err);

    // Test the assertOSError function using the OSErrorTask
    printf("=========== Testing OS assert ===========");



    
    

}

int main() {
    OS_ERR err;
    OSInit(&err);
    assertOSError(OS_MAIN_LOC, err);

    // Create the task manager thread
    OSTaskCreate(
        (OS_TCB*)&ManagerTaskTCB,
        (CPU_CHAR*)"Manager Task",
        (OS_TASK_PTR)ManagerTask,
        (void*)NULL,
        (OS_PRIO)1,
        (CPU_STK*)ManagerTaskStk,
        (CPU_STK_SIZE)128/10,
        (CPU_STK_SIZE)128,
        (OS_MSG_QTY)0,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    );
    assertOSError(OS_MAIN_LOC,err);

    OSStart(&err);

    while(1) {
        
    }
}
