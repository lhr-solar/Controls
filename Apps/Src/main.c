#include "common.h"
#include "config.h"
#include "os.h"
#include "BSP_UART.h"

#define TASK_PRINT_STK_SIZE 255
#define TASK_PRINT_PRIO 2

// Task control block for the application
static OS_TCB   TaskPrintTCB;
// Task stack
static CPU_STK  TaskPrintStk[TASK_PRINT_STK_SIZE];
// Task call function
static void TaskPrint (void *p_arg);

// For Task #2
static OS_TCB   TaskPrint2TCB;
static CPU_STK  TaskPrint2Stk[TASK_PRINT_STK_SIZE];
static void TaskPrint2(void *p_arg);

// Interrupt functions
#ifndef SIMULATION
static void __enable_irq() { asm("CPSIE I"); }
static void __disable_irq(){ asm("CPSID I"); }
#endif

int main(void) {
    OS_ERR err;

    #ifndef SIMULATION
    __disable_irq();
    #endif
    
    OSInit(&err);
    if(err != OS_ERR_NONE) {
        /* Something didn't work quite right. 
         * Spin for a while so the debugger can inspect.
         */
        volatile int x = 0;
        while(1) x++;
    }
    OSTaskCreate((OS_TCB     *)&TaskPrintTCB,
                 (CPU_CHAR   *)"Print Task",
                 (OS_TASK_PTR )TaskPrint,
                 (void       *)0,
                 (OS_PRIO     )TASK_PRINT_PRIO,
                 (CPU_STK    *)&TaskPrintStk[0],
                 (CPU_STK_SIZE)TASK_PRINT_STK_SIZE / 10,
                 (CPU_STK_SIZE)TASK_PRINT_STK_SIZE,
                 (OS_MSG_QTY  )0,
                 (OS_TICK     )0,
                 (void       *)0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR     *)&err);
    if(err != OS_ERR_NONE) {
        //Spin again
        volatile int y = 0;
        while(1) y++;
    }

    #ifndef SIMULATION
    // I'm not sure if this was done by the OS already, but just in case.
    __enable_irq();
    #endif
    
    OSStart(&err);
    
    // We should never reach this point...
    volatile int z = 0;
    while(1) { z++; }
    
    
    return 0;
}

void TaskPrint(void *p_arg) {
    OS_ERR err;

    #ifndef SIMULATION
    OS_CPU_SysTickInit(80000u); // Assuming an 16MHz clk, this gets us delays of 5ms
                                // This should be tweaked, since higher precision means
                                // larger overhead
    #else
    OS_CPU_SysTickInit();
    #endif

    BSP_UART_Init();

    OSTaskCreate((OS_TCB     *)&TaskPrint2TCB,
                 (CPU_CHAR   *)"Print Task",
                 (OS_TASK_PTR )TaskPrint2,
                 (void       *)0,
                 (OS_PRIO     )TASK_PRINT_PRIO,
                 (CPU_STK    *)&TaskPrint2Stk[0],
                 (CPU_STK_SIZE)TASK_PRINT_STK_SIZE / 10,
                 (CPU_STK_SIZE)TASK_PRINT_STK_SIZE,
                 (OS_MSG_QTY  )0,
                 (OS_TICK     )0,
                 (void       *)0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR     *)&err);

    int x = 0;
    while(1) {
        printf("This is working! x: %d\n\r", x++);
        OSTimeDly(100, OS_OPT_TIME_DLY, &err);
    }
}

void TaskPrint2(void *p_arg) {
    OS_ERR err;
    
    int y = 0;
    while(1) {
        printf("This is working! y: %d\n\r", y++);
        OSTimeDly(100, OS_OPT_TIME_DLY, &err);
    }
}
