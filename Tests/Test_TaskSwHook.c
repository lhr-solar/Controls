#include "Tasks.h"
#include "os.h"
#include "bsp.h"

//#define assert(expr) if (!(expr)) __asm("bkpt")
#define assert(expr)

static OS_TCB Task1TCB, Task2TCB;
static CPU_STK Task1Stk[DEFAULT_STACK_SIZE], Task2Stk[DEFAULT_STACK_SIZE];

void Task1(void *p_arg) {
    (void) p_arg;
    OS_ERR err;

    CPU_Init();
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U)OSCfg_TickRate_Hz);

    assert(PrevTask == NULL); // PrevTask should be null
    OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_HMSM_STRICT, &err);
    assert(PrevTask == &Task2TCB);
    while (1);

}

void Task2(void *p_arg) {
    assert(PrevTask == &Task1TCB);
    while (1);
}

int main(void) {
    OS_ERR err;
    OSInit(&err);
    TaskSwHook_Init();

    OSTaskCreate(
        (OS_TCB *)&Task1TCB,
        (CPU_CHAR *)"Task 1",
        (OS_TASK_PTR)Task1,
        (void *)NULL,
        (OS_PRIO)5,
        (CPU_STK *)Task1Stk,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE / 10,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)NULL,
        (void *)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR *)&err);

    OSTaskCreate(
        (OS_TCB *)&Task2TCB,
        (CPU_CHAR *)"Task 2",
        (OS_TASK_PTR)Task2,
        (void *)NULL,
        (OS_PRIO)6,
        (CPU_STK *)Task2Stk,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE / 10,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)NULL,
        (void *)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR *)&err);

    OSStart(&err);
}