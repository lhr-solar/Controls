/**
 * @file Test_Resource.h
 * @brief General test resources. Include this at the top of your test file.
 */

#include <assert.h>
#include "bsp.h"
#include "config.h"
#include "Tasks.h"

static void Init();

static OS_TCB TestTCB;
static CPU_STK TestStartTaskStk[DEFAULT_STACK_SIZE];

static void Test_Setup();
static void Test_Start(void *arg);

int main(){
    Init();
    while(1){}
    return 0;
}

static void Init(){
    OS_ERR err;
    
    // Disable interrupts
    __disable_irq();
    
    OSInit(&err);
    assertOSError(OS_MAIN_LOC, err);

    // Specific test setup instructions
    Test_Setup();

    // Initialize needed drivers
    BSP_UART_Init(UART_2);

    // Spawn the test task
    OSTaskCreate(
        (OS_TCB *)&TestTCB,
        (CPU_CHAR *)"Test Task",
        (OS_TASK_PTR)Test_Start,
        (void *)NULL,
        (OS_PRIO)12,
        (CPU_STK *)TestStartTaskStk,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)NULL,
        (void *)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR *)&err);
    assertOSError(OS_MAIN_LOC, err);

    // Enable interrupts
    __enable_irq();

    // Start OS
    OSStart(&err);
    assertOSError(OS_MAIN_LOC, err);

    // Start SysTick
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U) OSCfg_TickRate_Hz);
}