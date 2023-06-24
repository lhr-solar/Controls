/** 
 * @copyright Copyright (c) 2023 UT Longhorn Racing Solar
 * 
 * @file FaultLowPrio.c
 * @brief Fault Handler task spawn template for low-priority faults.
 * 
 * This contains a task spawned by error assertion functions to handle faults.
 * It pends on a mutex to ensure only one low-prio task is handled at a time,
 * Executes a callback function passed in as an argument,
 * And deletes itself once finished.
 * 
 * The goal of this system is to localize error handling
 * and allow high priority errors to be addressed as soon as possible
 * while also making sure there are no blocking calls in timer callbacks.
 * 
 * @author Madeleine Lee (KnockbackNemo)
*/

#include "Tasks.h"

/*** Macros ***/
#define TASK_FAULT_LOWPRIO_STK_SIZE DEFAULT_STACK_SIZE

/*** Variables ***/
static OS_TCB FaultLowPrio_TCB[3]; // Allows three low-prio errors at once
static CPU_STK FaultLowPrio_Stk[3]; // An arbitrary number, but we shouldn't reach it

static thread_counter = 0; // To keep track of which stack/tcb to use




bool Spawn_LowPrio_FaultThread(exception_t exception) {
    OS_ERR err;

    // Create the task with unique exception message/callback
    OSTaskCreate(
        (OS_TCB*)&FaultLowPrio_TCB[thread_counter],
        (CPU_CHAR*)"FaultLowPrio",
        (OS_TASK_PTR)Task_FaultLowPrio,
        (void*)((void*)&exception), // must be passed in as a void *
        (OS_PRIO)TASK_FAULT_LOWPRIO_PRIO, // Lower than init task but higher than others
        (CPU_STK*)&FaultLowPrio_Stk[thread_counter],
        (CPU_STK_SIZE)TASK_FAULT_LOWPRIO_STK_SIZE/10,
        (CPU_STK_SIZE)TASK_FAULT_LOWPRIO_STK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)0,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    );
    assertOSError(OS_FAULT_LOWPRIO_LOC, err);

    thread_counter = (thread_counter + 1) % 3; // tasks should get mutex in order
    // So we should be able to reuse task resources starting with the oldest


}

void Task_FaultLowPrio(void *p_arg) {
    OS_ERR err;


    // Ensure no other low-priority faults are already being handled
    // to avoid round-robin fault handling

    // Print the exception message

    // Run the callback function if it isn't null

    // Allow other low-priority fault threads to run

    // Fault handling thread has completed its job and will now self-destruct

    OSTaskDel(NULL, &err); // Delete this currently-running task
    assertOSError(OS_FAULT_LOWPRIO_LOC, err);

}