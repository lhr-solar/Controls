#include "os.h"
#include "Tasks.h"
#include "Minions.h"
#include <bsp.h>
#include "config.h"
#include "common.h"
#include "Contactors.h" 

static OS_TCB Task1TCB;
static CPU_STK Task1Stk[DEFAULT_STACK_SIZE];

void Task1(void *arg)
{   
    CPU_Init();
    
    BSP_UART_Init(UART_2);
    Minion_Init();
    Contactors_Init();

    OS_ERR err;

    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U)OSCfg_TickRate_Hz);

    // Initialize IgnitionContactor
    OSTaskCreate(
        (OS_TCB*)&IgnCont_TCB,
        (CPU_CHAR*)"IgnitionContactor",
        (OS_TASK_PTR)Task_Contactor_Ignition,
        (void*)NULL,
        (OS_PRIO)TASK_IGN_CONT_PRIO,
        (CPU_STK*)IgnCont_Stk,
        (CPU_STK_SIZE)WATERMARK_STACK_LIMIT,
        (CPU_STK_SIZE)TASK_IGN_CONT_STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)0,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    );
    assertOSError(OS_MAIN_LOC, err);

    while (1){
        printf("Array PBC: %d, Motor PBC: %d\n\r", Contactors_Get(ARRAY_PRECHARGE_BYPASS_CONTACTOR), Contactors_Get(MOTOR_CONTROLLER_BYPASS_PRECHARGE_CONTACTOR));

        OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &err);
    }
};

int main()
{
    OS_ERR err;
    OSInit(&err);

    // create tester thread
    OSTaskCreate(
        (OS_TCB *)&Task1TCB,
        (CPU_CHAR *)"Task 1",
        (OS_TASK_PTR)Task1,
        (void *)NULL,
        (OS_PRIO)13,
        (CPU_STK *)Task1Stk,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE / 10,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)NULL,
        (void *)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR *)&err);
    assertOSError(OS_MAIN_LOC, err);

    OSStart(&err);
}