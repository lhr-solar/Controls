/**
 * 
 * This file tests the fault state exception struct mechanism
 * to ensure thrown exceptions are handled correctly.
 * It does this by creating and faulting tasks 
 * to see if they stop working when an exception is thrown.
 * 
 * Choose the task to test using TEST_OPTION.
 * If testing ReadTritium non-Hall, choose the tritium error using READTRITIUM_OPTION.
 * Once the task enters an infinite loop, it won't ever exit and you'll need to restart the test.
 *
 * This test is run in LoopBack mode with all messages sent and received by the LeaderBoard.
 * However, it can be run in conjunction with motor-sim and car-sim
 * (which don't do anything) when simulated to appease Renode.
 * 
 * @file 
 * @author Madeleine Lee (madeleinercflee@utexas.edu)
 * @brief Tests the fault state exception mechanism
 * @version 
 * @date 2023-7-10
 *
 * @copyright Copyright (c) 2022 Longhorn Racing Solar
 *
 */

#include "BSP_UART.h"
#include "Tasks.h"
#include "CANbus.h"
#include "ReadTritium.h"
#include "Contactors.h"
#include "ReadCarCAN.h"
#include "UpdateDisplay.h"
#include "CANConfig.h"
#include "Minions.h"

enum { // Test menu enum
    TEST_GENERAL,
    TEST_OS_ASSERT,
    TEST_READTRITIUM,
    TEST_READCARCAN,
    TEST_UPDATEDISPLAY
};

/*** Constants ***/
#define TEST_OPTION TEST_READCARCAN // Decide what to test based on test menu enum
#define READTRITIUM_OPTION T_HALL_SENSOR_ERR // The enum for the tritium error we want to test (reference error enum)

/* READTRITIUM_OPTION menu:
    T_HARDWARE_OVER_CURRENT_ERR = (1<<0), 
    T_SOFTWARE_OVER_CURRENT_ERR = (1<<1), 
    T_DC_BUS_OVERVOLT_ERR = (1<<2), 
    T_HALL_SENSOR_ERR = (1<<3), 
    T_WATCHDOG_LAST_RESET_ERR = (1<<4), 
    T_CONFIG_READ_ERR = (1<<5), 
    T_UNDER_VOLTAGE_LOCKOUT_ERR = (1<<6), 
    T_DESAT_FAULT_ERR = (1<<7), 
    T_MOTOR_OVER_SPEED_ERR = (1<<8), 
    T_INIT_FAIL = (1<<9),
    T_NONE = 0x00,
*/


/*** Task components ***/
static OS_TCB ManagerTask_TCB;
static OS_TCB ExceptionTaskTCB;
static OS_TCB OSErrorTaskTCB;


static CPU_STK ManagerTask_Stk[DEFAULT_STACK_SIZE];
static CPU_STK ExceptionTaskStk[DEFAULT_STACK_SIZE];
static CPU_STK OSErrorTaskStk[DEFAULT_STACK_SIZE];





/*** Function prototypes ***/
void createExceptionTask(void * callback_function);
void createOSErrorTask(void);
void createReadCarCAN(void);
void createReadTritium(void);
void createUpdateDisplay(void);
void endTestReadCarCAN(void);

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

/**
 * @brief Creates and throws recoverable and nonrecoverable errors
 * with or without callback handlers to test the general assertion mechanism
 * @param test_callbacks the callback function to run during error handling
*/
void ExceptionTask(callback_t test_callbacks) {
    OS_ERR err;

    if (test_callbacks == NULL) {
        printf("\n\n\rTesting exceptions without callback functions");
    } else {
        printf("\n\n\rTesting exceptions with callback functions");
    }
    
    // Throw two recoverable errors
    printf("\n\n\rAsserting recoverable errors");
    // Throw an arbitrary recoverable error w/o locking the scheduler
    throwTaskError(OS_MAIN_LOC, 0x01, test_callbacks, OPT_NO_LOCK_SCHED, OPT_RECOV); 
    // Throw an arbitrary recoverable error w/ locked scheduler
    throwTaskError(OS_TASKS_LOC, 0x02, test_callbacks, OPT_LOCK_SCHED, OPT_RECOV); 
    
    // Throw a nonrecoverable error
    printf("\n\n\rAsserting a nonrecoverable error");
    throwTaskError(OS_SEND_CAN_LOC, 0x03, test_callbacks, OPT_NO_LOCK_SCHED, OPT_NONRECOV); //  Should still lock sched b/c nonrecoverable

    printf("\n\rTest failed: locked nonrecoverable error did not immediately result in an unrecoverable fault"); // Shouldn't print
    
    OSTaskDel(NULL, &err); // Self-delete task once finished, though we shouldn't get this far
    checkOSError(err);
}

// Test the assertOSError function by pending on a mutex that wasn't created
void OSErrorTask(void* arg) {
    OS_ERR err;
    OS_ERR test_err;
    OS_MUTEX testMut;
    CPU_TS ts;

    printf("\n\rasserting an OS error");
    OSMutexPend(&testMut, 0, OS_OPT_PEND_NON_BLOCKING, &ts, &test_err);
    assertOSError(OS_MAIN_LOC, test_err);
    printf("\n\rassertOSError test failed: assertion did not immediately result in an unrecoverable fault");
    
    OSTaskDel(NULL, &err); // Self-delete task once finished
    checkOSError(err);
}   

// Helper function to see the state of the contactors
static void print_Contactors() {
    printf("\n\rMotor PBC: %d", Contactors_Get(MOTOR_CONTROLLER_PRECHARGE_BYPASS_CONTACTOR));
    printf("\n\rArray PBC: %d", Contactors_Get(ARRAY_PRECHARGE_BYPASS_CONTACTOR));
}


// A high-priority task that manages other tasks and runs the tests
void Task_ManagerTask(void* arg) {
    OS_ERR err;
    CPU_Init();
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U) OSCfg_TickRate_Hz);
    CANbus_Init(MOTORCAN, NULL, 0);
    CANbus_Init(CARCAN, NULL, 0);
    Contactors_Init();
    Display_Init();
    UpdateDisplay_Init();  
    CANDATA_t canError;
    
    
    switch (TEST_OPTION) {
        case TEST_GENERAL:
            // Test the exceptions mechanism by creating and asserting recoverable and nonrecoverable errors
            // Successful if exception message and callback message are printed
            // and the fail message is not printed (tasks are stopped when they assert an error)
            printf("\n\n\r=========== Testing general task error assertion  ===========");

            // Expected error info
            // Current Error Location: 0x0008
            // Current Error Code: 0x0003
        
            // Test level 1 & 2 exceptions with callbacks
            createExceptionTask(exceptionCallback); 
            OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_HMSM_STRICT, &err);
            checkOSError(err);  
            break;

        case TEST_OS_ASSERT:
            // Test the assertOSError function using the OSErrorTask
            // Creates an OS error by pending on a mutex that isn't created
            // Successful if it doesn't print the fail message (task is stopped by asserting an error)
            printf("\n\n\r=========== Testing OS assert ===========");

            //Expected output: infinite while loop with no prints

            createOSErrorTask();
            OSTimeDlyHMSM(0, 0, 2, 0, OS_OPT_TIME_HMSM_STRICT, &err);
            checkOSError(err);
            break;

        case TEST_READTRITIUM:
            // Test exceptions in ReadTritium by sending the fault chosen in READTRITIUM_OPTION
            // Successful we enter a nonrecoverable fault and the fail message is not printed
            // And the correct error info is shown on the display
            // Also if the motor is reset three times before the fault for a hall error
            printf("\n\n\r=========== Testing ReadTritium ===========");

            // Expected error info
            // Current Error Location: 0x004 (OS_READ_TRITIUM_LOC)
            // Current Error Code: The value of READTRITIUM_OPTION

            // Error_ReadCarCAN: 0x0000
            // Error_ReadTritium: The value of READTRITIUM_OPTION
            // Error_UpdateDisplay: 0x0000

            uint16_t tritiumError = READTRITIUM_OPTION;
            *((uint16_t*)(&canError.data[4])) = tritiumError;
            canError.ID = MOTOR_STATUS;

            createReadTritium();
            printf("\n\n\rNow sending: %d", tritiumError); // Sending 0 means no Tritium error
            CANbus_Send(canError, CAN_BLOCKING, MOTORCAN);
            OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_HMSM_STRICT, &err); // Wait for ReadTritium to finish
            if (tritiumError == T_HALL_SENSOR_ERR) { // Send the message extra times to allow for motor restart
                for (int i = 0; i < 3; i++) { // Faults if greater than restart threshold (3)
                    CANbus_Send(canError, CAN_BLOCKING, MOTORCAN);
                    OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_HMSM_STRICT, &err);
                }  
            }

            // Fail message: this should not be reached because we should hit an unrecoverable fault first
            printf("\n\rError assertion did not lead to an unrecoverable fault");
            OSTaskDel(&ReadTritium_TCB, &err);
            checkOSError(err);   
        
            break;


        case TEST_READCARCAN:

            // Tests exceptions in ReadCarCAN by creating the tasks and sending messages
            // Successful if charging disable and missed messages turns off contactors
            // And we enter a nonrecoverable fault immediately after receiving the trip message
            /* Not tested on hardware due to dependencies on ReadCarCAN fix_ignition_delay branch changes */
            printf("\n\n\r=========== Testing ReadCarCAN ===========");              

            createReadCarCAN();  
            
            // Message for charging enable/disable
            CANDATA_t chargeMsg;
            // TODO: Will need to be tested post merge [ReadCarCAN --> FaultState]
            //chargeMsg.ID = CHARGE_ENABLE;
            chargeMsg.ID = BPS_CONTACTOR;
            *(uint64_t*)(&chargeMsg.data) = 0b001; 
            chargeMsg.idx = 0;
            printf("\n\rMade charging message");

            // Turn on array ignition
            printf("\n\rTurn Ignition to Array");
            Minions_Write(IGN_2, 1);                      // Ignition motor OFF
            Minions_Write(IGN_1, 0);                      // Ignition array ON
            if(Minions_Read(IGN_1) != false){             // Ensure ign array is ON
                printf("\n\rFailed to turn ign to array");
            }

            // Message for BPS Fault
            CANDATA_t tripBPSMsg;
            tripBPSMsg.ID = BPS_TRIP;
            *(uint64_t*)(&tripBPSMsg.data) = 0;
            tripBPSMsg.idx = 0;
            printf("\n\rMade BPS trip message");

            // Send enough charge enable messages
            chargeMsg.data[0] = true;
            for(int i = 0; i<7; i++){
                CANbus_Send(chargeMsg, CAN_BLOCKING,CARCAN);
                printf("\n\rSent enable chargeMsg %d", i);
                OSTimeDlyHMSM(0, 0, 0, 400, OS_OPT_TIME_HMSM_STRICT, &err);
                checkOSError(err);
                printf("\n\rChargeEnable: %d", ChargeEnable_Get());
            }

            print_Contactors();

            printf("\n\n\rSending Charge Disable Msg");
            // Send charge disable messages to test charge disable contactor callback
            // Fault state should turn off the array PBC but not enter a nonrecoverable fault
            chargeMsg.data[0] = false;
            for(int i = 0; i<5; i++){
                CANbus_Send(chargeMsg, CAN_BLOCKING,CARCAN);
                printf("\n\rSent disable chargeMsg %d", i);
                OSTimeDlyHMSM(0, 0, 0, 400, OS_OPT_TIME_HMSM_STRICT, &err);
                checkOSError(err);
                printf("\n\rChargeEnable: %d", ChargeEnable_Get());
                printf("\n\rArray PBC %d", Contactors_Get(ARRAY_PRECHARGE_BYPASS_CONTACTOR));
                printf("\n\rMotor PBC %d", Contactors_Get(MOTOR_CONTROLLER_PRECHARGE_BYPASS_CONTACTOR));
            }

            print_Contactors();

            printf("\n\n\rSending Charge Enable Msg");
            // Send more charge enable messages so the contactor gets flipped on again
            chargeMsg.data[0] = true;
            for(int i = 0; i < 7; i++){
                CANbus_Send(chargeMsg, CAN_BLOCKING,CARCAN);
                printf("\n\rSent enable chargeMsg %d", i);
                OSTimeDlyHMSM(0, 0, 0, 400, OS_OPT_TIME_HMSM_STRICT, &err);
                checkOSError(err);
                printf("\n\rChargeEnable: %d", ChargeEnable_Get());
            }

            print_Contactors();

            printf("\n\n\rTrigger canWatchTimer");
            // Pause the delivery of messages to trigger the canWatchTimer
            for(int i = 0; i<5; i++){
                printf("\n\rDelay %d", i);
                OSTimeDlyHMSM(0, 0, 0, 400, OS_OPT_TIME_HMSM_STRICT, &err);
                checkOSError(err);
                printf("\n\rChargeEnable: %d", ChargeEnable_Get());
                printf("\n\rArray PBC %d", Contactors_Get(ARRAY_PRECHARGE_BYPASS_CONTACTOR));
                printf("\n\rMotor PBC %d", Contactors_Get(MOTOR_CONTROLLER_PRECHARGE_BYPASS_CONTACTOR));
            } 

            // Check the contactors
            print_Contactors();

            // Turn on contactors so we can see if they get turned off by the error assertion
            Contactors_Set(MOTOR_CONTROLLER_PRECHARGE_BYPASS_CONTACTOR, ON, true);
            Contactors_Set(ARRAY_PRECHARGE_BYPASS_CONTACTOR, ON, true); // Although BPS has control of the precharge contactor

            print_Contactors(); // See the state of the contactors before we send the trip message

            printf("\n\n\rBPS Trip");
            // Send a trip message of 1 (trip)
            *(uint64_t*)(&tripBPSMsg.data) = 1;
            CANbus_Send(tripBPSMsg, CAN_BLOCKING, CARCAN);
            printf("\n\rSent trip message %d", tripBPSMsg.data[0]);
            OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &err);

            // Code below here shouldn't be run unless the error assertion fails

            // By now, the BPS Trip message should have been sent and the contactors should be off
            print_Contactors();
            endTestReadCarCAN(); // Delete ReadCarCAN and UpdateDisplay tasks
            break;


        case TEST_UPDATEDISPLAY: 
            // Call update display put next to overflow the queue
            printf("\n\n\r=========== Test UpdateDisplay ===========");
            
            // Expected output
            // The display should reset, and then the test will hit an infinite while loop.
            createUpdateDisplay();

            OSTimeDlyHMSM(0, 0, 10, 0, OS_OPT_TIME_HMSM_STRICT, &err); // Wait for the display to initialize
            printf("\n\rSending display messages to fill queue");
            
            for (int i = 0; i < 10; i++) {
                UpdateDisplay_SetCruiseState(1 + i); 
            }
            OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_HMSM_STRICT, &err);
            checkOSError(err);
            OSTaskDel(&UpdateDisplay_TCB, &err);
            checkOSError(err);
            break;
    }

    while(1){
        printf("\n\rReached end of test.");
        OSTimeDlyHMSM(0, 0, 5, 0, OS_OPT_TIME_HMSM_STRICT, &err);
    }
}

// Task creation functions

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
    printf("\n\rCreated ReadTritium");
    createUpdateDisplay();   
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
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE/10,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    );
    checkOSError(err);
    printf("\n\rCreated readCarCAN"); 
    createUpdateDisplay();   

}

// Deletes tasks used in ReadCarCAN test
void endTestReadCarCAN(void) {
    OS_ERR err;
    OSTaskDel(&ReadCarCAN_TCB, &err);
    checkOSError(err);
    OSTaskDel(&UpdateDisplay_TCB, &err);
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
    printf("\n\rCreated and initialized Display and UpdateDisplay"); 

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
    printf("\n\rCreated ExceptionTask");   

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
    printf("\n\rCreated OSErrorTask");   

}

int main(void) {

    OS_ERR err;
    BSP_UART_Init(UART_2);

    OSInit(&err);

    checkOSError(err);  
    
    TaskSwHook_Init();
    
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

