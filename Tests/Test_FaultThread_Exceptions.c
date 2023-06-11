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
#include "CANbus.h"



/*** Task components ***/
static OS_TCB ManagerTask_TCB;
static OS_TCB ExceptionTaskTCB;
static OS_TCB OSErrorTaskTCB;
static CPU_STK ManagerTask_Stk;
static CPU_STK ExceptionTaskStk;
static CPU_STK OSErrorTaskStk;

/*** Constants ***/
#define READY_ID 0x123


// To do:
// Change all task initializations to be consistent in constant usage if both options work
// Use a semaphore instead of doing a full blind wait
// Make a different OS Error function
// change bool to semaphore?

// Assertion function for OS errors
void checkOSError(OS_ERR err) {
    if (err != OS_ERR_NONE) {
        printf("OS error code %d\n", err);
    } else {
        printf("\n\rNo error yet");
    }
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

    while(1){};
}

// Test the assertOSError function by pending on a mutex that wasn't created
void OSErrorTask(void* arg) {
    OS_ERR err;
    OS_MUTEX testMut;
    CPU_TS ts;
    OSMutexPend(&testMut, 0, OS_OPT_PEND_NON_BLOCKING, &ts, &err);
    assertOSError(OS_MAIN_LOC, err);
    printf("assertOSError test failed: assertion did not immediately result in an unrecoverable fault");
    while(1){};
}   


// Task creation functions

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
    checkOSError(err); 

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
    checkOSError(err);
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
    checkOSError(err);

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
    checkOSError(err); 

}

// Creates a task to test exceptions on their own
// callback_function: pass in the callback function to use or NULL for no callbacks
void createExceptionTask(void * callback_function) {
    OS_ERR err;
     
     OSTaskCreate(
        (OS_TCB *)&ExceptionTaskTCB,
        (CPU_CHAR *)"ExceptionTask",
        (OS_TASK_PTR)ExceptionTask,
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
    checkOSError(err); 

}

// Creates a task to test the assertOSError function
void createOSErrorTask(void) {
    OS_ERR err;
     
     OSTaskCreate(
        (OS_TCB *)&OSErrorTaskTCB,
        (CPU_CHAR *)"assertOSError Task",
        (OS_TASK_PTR)OSErrorTask,
        (void *)NULL,
        (OS_PRIO)7,
        (CPU_STK *)OSErrorTaskStk,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE / 10,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)NULL,
        (void *)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR *)&err
    );
    checkOSError(err); 

}





// A high-priority task that manages other tasks and runs the tests
void Task_ManagerTask(void* arg) {
    printf("\n\rIn manager task");
    CPU_Init();
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U) OSCfg_TickRate_Hz);
    OS_ERR err;
    CANbus_Init(MOTORCAN, NULL, 0);
 


    while (1) {
    // Test the exceptions mechanism by creating and throwing exceptions of priorities 1 and 2
    // Both with and without callback functions
    printf("=========== Testing exception priorities 1 and 2 ===========");
   
    createExceptionTask(NULL); // Test level 1 & 2 exceptions without callbacks
    checkOSError(err); 
    OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &err); // Wait for the task to finish
    checkOSError(err); 
    OSTaskDel(&ExceptionTaskTCB, &err);
    checkOSError(err); 

    createExceptionTask(exceptionCallback); // Test level 1 & 2 exceptions with callbacks
    checkOSError(err); 
    OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &err);
    checkOSError(err); 
    OSTaskDel(&ExceptionTaskTCB, &err);
    checkOSError(err); 

    // Test the assertOSError function using the OSErrorTask
    printf("=========== Testing OS assert ===========");

    createOSErrorTask();
    checkOSError(err); 
    OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &err);
    checkOSError(err); 

    // Test individual tasks error assertions
    printf("=========== Testing ReadTritium ===========");

    CANDATA_t canMessage = {0};

    createFaultState();
    checkOSError(err); 
    createReadTritium();
    checkOSError(err); 
    // Send a message to the motor to tell it to start sending stuff
    canMessage.ID = READY_ID;
    canMessage.idx = 0;
    *(uint16_t*)(&canMessage.data) =  0x4444;
    CANbus_Send(canMessage, CAN_BLOCKING, MOTORCAN);
    OSTimeDlyHMSM(0, 0, 5, 0, OS_OPT_TIME_HMSM_STRICT, &err); // Wait for ReadTritium to finish
    OSTaskDel(&ReadTritium_TCB, &err);




    }







    
    

}

int main(void) {
    OS_ERR err;
    OSInit(&err);
    //assertOSError(OS_MAIN_LOC, err);
    printf("\n\rThing before BSP UART Init");
    BSP_UART_Init(UART_2);
    printf("\n\rThing after bsp uart init");

    // Create the task manager thread
    OSTaskCreate(
        (OS_TCB*)&ManagerTask_TCB,
        (CPU_CHAR*)"Manager Task",
        (OS_TASK_PTR)Task_ManagerTask,
        (void*)NULL,
        (OS_PRIO)1,
        (CPU_STK*)ManagerTask_Stk,
        (CPU_STK_SIZE)128/10,
        (CPU_STK_SIZE)128,
        (OS_MSG_QTY)0,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    );
    checkOSError(err);

    OSStart(&err);
    checkOSError(err);

    while(1) {
        printf("\n\rInside main while loop wheeeee");
        for (int i = 0; i < 99999; i++){}
        printf("\n\rLet's try a time delay too");
        OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &err);
        
    }
}
