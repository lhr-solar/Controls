#include "common.h"
#include "config.h"
#include "Tasks.h"
#include <os.h>

static OS_TCB Task1_TCB;
static OS_TCB Task2_TCB;
static CPU_STK Task1_Stk[128];
static CPU_STK Task2_Stk[128];


void Task1(void* p_arg) {
    OS_ERR err;
    CPU_TS ts;
    OSTaskSemPend(0, OS_OPT_PEND_BLOCKING, &ts, &err);
    printf("hello\n");
    OSTaskDel(NULL, &err);
}

void Task2(void *p_arg) {
    OS_ERR err;
    CPU_Init();
    OS_CPU_SysTickInit();
    printf("Arg is : %d\n", (int)p_arg);

    OSTimeDlyHMSM(0, 0, 5, 0, OS_OPT_TIME_HMSM_STRICT, &err);
    OSTaskSemPost(&Task1_TCB, OS_OPT_POST_NONE, &err);
    OSTaskDel(NULL, &err);
}

void main(void) {
    OS_ERR err;
    OSInit(&err);

    // Initialized with error
    if (err != OS_ERR_NONE) {
        printf("OS error code %d\n", err);
    }

    INIT_TASK(Task1, 5, NULL, err);

    INIT_TASK(Task2, 4, 42, err);

    // Task not created
    if (err != OS_ERR_NONE) {
        printf("Task error code %d\n", err);
    }

    OSStart(&err);
    if (err != OS_ERR_NONE) {
        printf("OS error code %d\n", err);
    }


}