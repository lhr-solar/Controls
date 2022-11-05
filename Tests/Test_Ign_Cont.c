#include "os.h"
#include "Tasks.h"
#include "Minions.h"
#include <bsp.h>
#include "config.h"
#include "common.h"
#include "Contactors.h"


int main(void)
{
    OS_ERR err;
    OSInit(&err);
    Contactors_Init();
    Minion_Init();
    CPU_Init();
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

    OSStart(&err);
}