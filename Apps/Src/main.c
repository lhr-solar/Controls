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

// Interrupt functions
static void __enable_irq() { asm("CPSIE I"); }
static void __disable_irq(){ asm("CPSID I"); }

int main(void) {
    OS_ERR err;

    __disable_irq();
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

    // I'm not sure if this was done by the OS already, but just in case.
    __enable_irq();
    
    OSStart(&err);
    
    // We should never reach this point...
    volatile int z = 0;
    while(1) { z++; }
    
    
    return 0;
}

void TaskPrint(void *p_arg) {
    while(1) {
        printf("This is working!");
    }
}