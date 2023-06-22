/**
 * 
 * This file tests the fault state exception struct mechanism
 * to ensure thrown exceptions are handled correctly.
 * It does this by testing the assertion functions for each priority option
 * and creating and faulting tasks using the car and bps sim on Renode
 *
 * This test is run in conjunction with Test_motor_FaultThread_Exceptions
 * and Test_car_FaultThread_Exceptions on Renode
 * 
 * @file 
 * @author Madeleine Lee (madeleinercflee@utexas.edu)
 * @brief Tests the fault state exception mechanism
 * @version 
 * @date 2023-6-08
 *
 * @copyright Copyright (c) 2022 Longhorn Racing Solar
 *
 */

#include "BSP_UART.h"
#include "FaultState.h"
#include "Tasks.h"
#include "CANbus.h"
#include "ReadTritium.h"
#include "Contactors.h"
#include "ReadCarCAN.h"
#include "UpdateDisplay.h"
#include "CANConfig.h"

/*** Constants ***/
#define READY_MSG 0x4444
#define STACK_SIZE 128


/*** Task components ***/
static OS_TCB ManagerTask_TCB;
static OS_TCB ExceptionTaskTCB;
static OS_TCB OSErrorTaskTCB;
//static OS_TCB BPSSim_TCB;
//static OS_TCB MotorSim_TCB;


static CPU_STK ManagerTask_Stk[DEFAULT_STACK_SIZE];
static CPU_STK ExceptionTaskStk[DEFAULT_STACK_SIZE];
static CPU_STK OSErrorTaskStk[DEFAULT_STACK_SIZE];
//static CPU_STK BPSSim_Stk[STACK_SIZE];
//static CPU_STK MotorSim_Stk[STACK_SIZE];




/*** Globals ***/
OS_SEM TestReady_Sema4;
OS_SEM TestDone_Sema4;


// Assertion function for OS errors
void checkOSError(OS_ERR err) {
    if (err != OS_ERR_NONE) {
        printf("OS error code %d\n", err);
        __asm("bkpt");
    }
}
    


// Callback function for ExceptionTask test exceptions
void exceptionCallback(void) {
    printf("\n\rException callback successfully executed.");
}


// Creates an exception of priority 1 and 2 to test
// test_callbacks: the callback function to use for the exceptions
void ExceptionTask(void* test_callbacks) {
    OS_ERR err;

    if (test_callbacks == NULL) {
        printf("\n\n\rTesting exceptions without callback functions");
    } else {
        printf("\n\n\rTesting exceptions with callback functions");
    }
    // Throw a priority 2 exception
    printf("\n\n\rThrowing priority level 2 exception");
    exception_t prio2Exception = {.prio = PRI_RECOV, .message = "\n\rprio2 exception message", .callback = test_callbacks};
    assertExceptionError(prio2Exception);
    
    // Throw a priority 1 exception
    printf("\n\n\rThrowing priority level 1 exception");
    exception_t prio1Exception = {.prio = PRI_NONRECOV, .message = "\n\rprio1 exception message", .callback = test_callbacks};
    OSSemPost(&TestReady_Sema4, OS_OPT_POST_1, &err); // Alert manager task that the test is almost finished
    assertExceptionError(prio1Exception);
    printf("\n\rTest failed: Prio 1 exception did not immediately result in an unrecoverable fault");

    while(1){};
}

// Test the assertOSError function by pending on a mutex that wasn't created
void OSErrorTask(void* arg) {
    OS_ERR err;
    OS_ERR test_err;
    OS_MUTEX testMut;
    CPU_TS ts;
    printf("\n\rasserting an OS error");
    OSMutexPend(&testMut, 0, OS_OPT_PEND_NON_BLOCKING, &ts, &test_err);
    OSSemPost(&TestReady_Sema4, OS_OPT_POST_1, &err);
    assertOSError(OS_MAIN_LOC, test_err);
    printf("\n\rassertOSError test failed: assertion did not immediately result in an unrecoverable fault");
    while(1){};
}   


/* void MotorSim(void *p_arg){
    printf("\n\rIn MotorSim");
    OS_ERR err;
    //CANDATA_t dataBuf = {0};
    //uint16_t data = 0;

    CANDATA_t canError = {0};
    canError.ID=MOTOR_STATUS;


    for (int i = 0; i < NUM_TRITIUM_ERRORS + 2; i++) {
         

        OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_HMSM_STRICT, &err);
        uint16_t tritiumError = (0x01<<i)>>1;
        *((uint16_t*)(&canError.data[4])) = tritiumError;
        printf("\n\rNow sending: %d", tritiumError);

         // Send error messages to the leader board
        if (tritiumError == T_HALL_SENSOR_ERR) { // Send the message extra times to allow for motor restart
            CANbus_Send(canError, CAN_BLOCKING, MOTORCAN);
            OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_HMSM_STRICT, &err);
            CANbus_Send(canError, CAN_BLOCKING, MOTORCAN);
            OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_HMSM_STRICT, &err);
        }

        CANbus_Send(canError, CAN_BLOCKING, MOTORCAN);
        OSSemPost(&TestReady_Sema4, OS_OPT_POST_1, &err); // Alert manager task that the test is almost finished
   
        
    }



}
 */

  

/* 

void BPSSim(void *p_arg){

    OS_ERR err;

    //CANDATA_t dataBuf; // A buffer in which we can store the messages we read
    //uint16_t data = 0;

    //  do {
    //          ErrorStatus status = CANbus_Read(&dataBuf, false, MOTORCAN);
    //          if (status != SUCCESS) {
    //              printf("\n\rCANbus Read error status: %d\n\r", status);
    //          } else {
    //              data = *((uint16_t*)(&dataBuf.data[0]));
    //              printf("\n\rWaiting for start. Read: %d", data);
    //          }
    //          OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &err);
    //      } while (data != READY_MSG);

    CANDATA_t chargeMsg;
    chargeMsg.ID = CHARGE_ENABLE;
    *(uint64_t*)(&chargeMsg.data) = 0;
    chargeMsg.idx = 0;

    // Message for BPS Fault
    CANDATA_t tripBPSMsg;
    tripBPSMsg.ID = BPS_TRIP;
    *(uint64_t*)(&tripBPSMsg.data) = 0;
    tripBPSMsg.idx = 0;

    


    // Send five charge enable messages so the contactors get flipped on
    chargeMsg.data[0] = true;
    for(int i = 0; i<3; i++){
        CANbus_Send(chargeMsg, CAN_BLOCKING,CARCAN);
        printf("\nSent enable chargeMsg %d", i);
        OSTimeDlyHMSM(0, 0, 0, 400, OS_OPT_TIME_HMSM_STRICT, &err);
    }

    // Send five charge disable messages to test prio-2 disable contactor callback
    // Fault state should turn off contactors but not enter a nonrecoverable fault
    chargeMsg.data[0] = false;
    for(int i = 0; i<3; i++){
        CANbus_Send(chargeMsg, CAN_BLOCKING,CARCAN);
        printf("\nSent disable chargeMsg %d", i);
        OSTimeDlyHMSM(0, 0, 0, 400, OS_OPT_TIME_HMSM_STRICT, &err);
    }

    // Send five more charge enable messages so the contactors get flipped on again
    chargeMsg.data[0] = true;
    for(int i = 0; i<3; i++){
        CANbus_Send(chargeMsg, CAN_BLOCKING,CARCAN);
        printf("\nSent enable chargeMsg %d", i);
        OSTimeDlyHMSM(0, 0, 0, 400, OS_OPT_TIME_HMSM_STRICT, &err);
    }

    // Send a trip message of 1 (trip)
    // Note: trip messages are not usually sent,
    // so any trip message (even 0) should be a concern.
    // Maybe we should handle them differently?
    *(uint64_t*)(&tripBPSMsg.data) = 1;
    CANbus_Send(tripBPSMsg, CAN_BLOCKING, CARCAN);
    printf("\nSent trip message %d", tripBPSMsg.data[0]);

    //dataBuf.ID = CARDATA;
    *((uint16_t*)(&dataBuf.data[0])) = READY_MSG;
    //CANbus_Send(dataBuf, CAN_NON_BLOCKING, CARCAN);


    while(1){};
}
     */







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

/* // The bps-sim task moved here as BPSSim for Loopback mode testing
void createBPSSimTask(void) {
    OS_ERR err;

        OSTaskCreate(
        (OS_TCB*)&BPSSim_TCB,
        (CPU_CHAR*)"bps-sim",
        (OS_TASK_PTR)BPSSim,
        (void*)NULL,
        (OS_PRIO)4, // May need to change this
        (CPU_STK*)BPSSim_Stk,
        (CPU_STK_SIZE)STACK_SIZE/10,
        (CPU_STK_SIZE)STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR|OS_OPT_TASK_STK_CHK),
        (OS_ERR*)&err
    );
    checkOSError(err); 

    
    if (err != OS_ERR_NONE) {
        printf("bps sim error code %d\n\r", err);
    }
} */

/* 
void createMotorSimTask(void){
    OS_ERR err;

    OSTaskCreate(
        (OS_TCB*)&MotorSim_TCB,
        (CPU_CHAR*)"Task1",
        (OS_TASK_PTR)MotorSim,
        (void*)NULL,
        (OS_PRIO)4, // May need to change this
        (CPU_STK*)MotorSim_Stk,
        (CPU_STK_SIZE)STACK_SIZE/10,
        (CPU_STK_SIZE)STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR|OS_OPT_TASK_STK_CHK),
        (OS_ERR*)&err
    );

    if (err != OS_ERR_NONE) {
        printf("Task error code %d\n", err);
    }
}

 */



// A high-priority task that manages other tasks and runs the tests
void Task_ManagerTask(void* arg) {
    OS_ERR err;
    CPU_TS ts;
    CPU_Init();
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U) OSCfg_TickRate_Hz);
    CANbus_Init(MOTORCAN, NULL, 0);
    CANbus_Init(CARCAN, NULL, 0);
    Contactors_Init();
    OSSemCreate(&TestReady_Sema4, "Ready Flag Semaphore", 0, &err);
    //ErrorStatus errorCode;
    
    /*** test */
 

    // CANDATA_t testmsg = {0};
    // testmsg.ID=MOTOR_STATUS;
    // testmsg.idx=0;
    // *((uint16_t*)(&testmsg.data[0])) = 4444;
    
    // while (1) {
    //     CANbus_Send(testmsg, CAN_BLOCKING, MOTORCAN);
    // }

    /****/

    while (1) {
        // Test the exceptions mechanism by creating and throwing exceptions of priorities 1 and 2
        // Both with and without callback functions
        // Successful if exception message and callback message (if applicable) are printed
        // and the fail message is not printed (tasks are stopped when they assert an error)
        printf("\n\n\r=========== Testing exception priorities 1 and 2 ===========");
    
        // Test level 1 & 2 exceptions without callbacks
        createExceptionTask(NULL); 
        checkOSError(err); 
        createFaultState();
        checkOSError(err); 
        OSSemPend(&TestReady_Sema4, 0, OS_OPT_PEND_BLOCKING, &ts, &err); // Wait for task to finish
        checkOSError(err); 
        OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_HMSM_STRICT, &err); // extra time for the task finished
        checkOSError(err); 
        OSTaskDel(&ExceptionTaskTCB, &err);
        checkOSError(err); 
        OSTaskDel(&FaultState_TCB, &err);
        checkOSError(err); 
        
        // Test level 1 & 2 exceptions with callbacks
        createExceptionTask(exceptionCallback); 
        checkOSError(err); 
        createFaultState();
        checkOSError(err); 
        OSSemPend(&TestReady_Sema4, 0, OS_OPT_PEND_BLOCKING, &ts, &err);
        OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_HMSM_STRICT, &err);
        checkOSError(err); 
        OSTaskDel(&ExceptionTaskTCB, &err);
        checkOSError(err); 
        OSTaskDel(&FaultState_TCB, &err);
        checkOSError(err); 

        // Test the assertOSError function using the OSErrorTask
        // Creates an OS error by pending on a mutex that isn't created
        // Successful if it prints the OS Error code
        // and doesn't print the fail message (task is stopped by asserting an error)
        printf("\n\n\r=========== Testing OS assert ===========");

        createOSErrorTask();
        checkOSError(err); 
        createFaultState();
        checkOSError(err); 
        OSSemPend(&TestReady_Sema4, 0, OS_OPT_PEND_BLOCKING, &ts, &err);
        checkOSError(err);
        OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &err);
        checkOSError(err); 
        OSTaskDel(&OSErrorTaskTCB, &err);
        checkOSError(err); 
        OSTaskDel(&FaultState_TCB, &err);

        // Test exceptions in ReadTritium by creating the tasks
        // and sending faults using the motor sim on Renode
        printf("\n\n\r=========== Testing ReadTritium ===========");

        //CANDATA_t dataBuf = {0};
        //uint16_t data = 0;

        CANDATA_t canError = {0};
        canError.ID=MOTOR_STATUS;


        for (int i = 0; i < NUM_TRITIUM_ERRORS + 2; i++) {
            uint16_t tritiumError = (0x01<<i)>>1; // Change to i-1?
            *((uint16_t*)(&canError.data[4])) = tritiumError;

            createFaultState();
            createReadTritium();
            printf("\n\rNow sending: %d", tritiumError);
            CANbus_Send(canError, CAN_BLOCKING, MOTORCAN);
            OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_HMSM_STRICT, &err); // Wait for ReadTritium to finish
            printf("\n\rShould be testing Tritium error %d", i);
             if (tritiumError == T_HALL_SENSOR_ERR) { // Send the message extra times to allow for motor restart
                CANbus_Send(canError, CAN_BLOCKING, MOTORCAN);
                printf("\n\rShould be testing Tritium error %d", i);
                OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_HMSM_STRICT, &err);
                CANbus_Send(canError, CAN_BLOCKING, MOTORCAN);
                printf("\n\rShould be testing Tritium error %d", i);
                OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_HMSM_STRICT, &err);
            }
            
            OSTaskDel(&ReadTritium_TCB, &err);
            checkOSError(err);
            OSTaskDel(&FaultState_TCB, &err);
            checkOSError(err);

        }

        // Tests exceptions in ReadCarCAN by creating the tasks
        // and sending messages using the bps sim on Renode
        printf("\n\r=========== Testing ReadCarCAN ===========");
        createFaultState();
        checkOSError(err);
        createReadCarCAN();
        //createBPSSimTask();
        checkOSError(err);
        //CANbus_Send(canMessage, CAN_BLOCKING, CARCAN);
        //uint16_t canMsgData = 0;

        
    CANDATA_t chargeMsg;
    chargeMsg.ID = CHARGE_ENABLE;
    *(uint64_t*)(&chargeMsg.data) = 0;
    chargeMsg.idx = 0;

    // Message for BPS Fault
    CANDATA_t tripBPSMsg;
    tripBPSMsg.ID = BPS_TRIP;
    *(uint64_t*)(&tripBPSMsg.data) = 0;
    tripBPSMsg.idx = 0;

    


    // Send five charge enable messages so the contactors get flipped on
    chargeMsg.data[0] = true;
    for(int i = 0; i<3; i++){
        CANbus_Send(chargeMsg, CAN_BLOCKING,CARCAN);
        printf("\nSent enable chargeMsg %d", i);
        OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &err);
        printf("\n\rChargeEnable: %d", ChargeEnable_Get());
    }

    // Send five charge disable messages to test prio-2 disable contactor callback
    // Fault state should turn off contactors but not enter a nonrecoverable fault
    chargeMsg.data[0] = false;
    for(int i = 0; i<3; i++){
        CANbus_Send(chargeMsg, CAN_BLOCKING,CARCAN);
        printf("\nSent disable chargeMsg %d", i);
        OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &err);
        printf("\n\rChargeEnable: %d", ChargeEnable_Get());
    }
    
    for(int i = 0; i<2; i++){
        printf("\nDelay %d", i);
        OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &err);
        printf("\n\rChargeEnable: %d", ChargeEnable_Get());
    }

    // Send five more charge enable messages so the contactors get flipped on again
    chargeMsg.data[0] = true;
    for(int i = 0; i<5; i++){
        CANbus_Send(chargeMsg, CAN_BLOCKING,CARCAN);
        printf("\nSent enable chargeMsg %d", i);
        OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &err);
        printf("\n\rChargeEnable: %d", ChargeEnable_Get());
    }
    printf("\n\rMotor contactor: %d", Contactors_Get(MOTOR_CONTACTOR));// Check the contactors
    printf("\n\rArray_precharge contactor: %d", Contactors_Get(ARRAY_PRECHARGE));
    printf("\n\rArray contactor %d", Contactors_Get(ARRAY_CONTACTOR));

    // Send a trip message of 1 (trip)
    // Note: trip messages are not usually sent,
    // so any trip message (even 0) should be a concern.
    // Maybe we should handle them differently?
    *(uint64_t*)(&tripBPSMsg.data) = 1;
    CANbus_Send(tripBPSMsg, CAN_BLOCKING, CARCAN);
    printf("\nSent trip message %d", tripBPSMsg.data[0]);

        // By now, the BPS Trip message should have been sent
        //OSTimeDlyHMSM(0, 0, 3, 0, OS_OPT_TIME_HMSM_STRICT, &err); // Give ReadCarCAN time to turn off contactors
        printf("\n\rMotor contactor: %d", Contactors_Get(MOTOR_CONTACTOR));// Check the contactors
        printf("\n\rArray_precharge contactor: %d", Contactors_Get(ARRAY_PRECHARGE));
        printf("\n\rArray contactor %d", Contactors_Get(ARRAY_CONTACTOR));

        OSTaskDel(&ReadCarCAN_TCB, &err);
        checkOSError(err);
        OSTaskDel(&FaultState_TCB, &err);
        checkOSError(err);

        printf("\n\n\r=========== Test UpdateDisplay ===========");
        // Might be able to test if we UpdateDisplay_Init() and then make the task
        // UpdateDisplay and Display error functions do actually do the same thing, so one of them should be deleted.
        //UpdateDisplay_Init();
        // Call update display put next to overflow the queue (actual issue)
       
        // Updating display without updateDisplayInit()
        createUpdateDisplay();
        createFaultState();
        OSTimeDlyHMSM(0, 0, 3, 0, OS_OPT_TIME_HMSM_STRICT, &err);
        checkOSError(err);
        OSTaskDel(&UpdateDisplay_TCB, &err);
        checkOSError(err);
        OSTaskDel(&FaultState_TCB, &err);
        checkOSError(err);

        // overflow the queue


    }

}

int main(void) {

    OS_ERR err;
    BSP_UART_Init(UART_2);

    OSInit(&err);

    checkOSError(err);
    
    
    // Create the task manager thread
    OSTaskCreate(
        (OS_TCB*)&ManagerTask_TCB,
        (CPU_CHAR*)"Manager Task",
        (OS_TASK_PTR)Task_ManagerTask,
        (void*)NULL,
        (OS_PRIO)1,
        (CPU_STK*)ManagerTask_Stk,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE / 10,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    ); 
    

    checkOSError(err);

    OSStart(&err);
    checkOSError(err);

    while(1) {}
}

