/**
 * @file Test.c
 * @brief Tests static behavior of file.c (unit testing)
 */

#include <assert.h>
#include "bsp.h"
#include "config.h"
#include "Tasks.h"

static void Init();

static OS_TCB StartTaskTCB;
static CPU_STK StartTaskStk[DEFAULT_STACK_SIZE];
static void StartTask();

int main(){
    Init();
    while(1){}
}

static void Init(){
    // Initialize any drivers in this method
    BSP_UART_Init(UART_2);
    CPU_Init();

    OS_ERR err;
    // Spawn the test task
    OSTaskCreate(
        (OS_TCB *)&StartTaskTCB,
        (CPU_CHAR *)"Start Task",
        (OS_TASK_PTR)StartTask,
        (void *)NULL,
        (OS_PRIO)12,
        (CPU_STK *)StartTaskStk,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)NULL,
        (void *)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR *)&err);
    assertOSError(OS_MAIN_LOC, err);
}


static void StartTask(void *arg){
    // Spawn any required applications here.
    
    
    // Run any test functions here.

    return 0;
}