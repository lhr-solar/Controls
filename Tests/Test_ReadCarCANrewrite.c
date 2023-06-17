#include "Tasks.h"
#include "CANConfig.h"

static OS_TCB Task1TCB;
static CPU_STK Task1Stk[DEFAULT_STACK_SIZE];

void Task1(void *arg)
{
    OS_ERR err;

    CPU_Init();
    
    // Initialize drivers
    BSP_UART_Init(UART_2);
    CANbus_Init(CARCAN, (CANId_t*)carCANFilterList, NUM_CARCAN_FILTERS);
    CANbus_Init(MOTORCAN, NULL, NUM_MOTORCAN_FILTERS);
    Contactors_Init();
    Display_Init();
    Minion_Init();
    CAN_Queue_Init();

    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U)OSCfg_TickRate_Hz);

    OSTaskCreate(
        (OS_TCB*)&ReadCanCAN_TCB,
        (CPU_CHAR*)"ReadCarCAN",
        (OS_TASK_PTR)Task_ReadCarCAN,
        (void*) NULL,
        (OS_PRIO)TASK_READ_CAR_CAN_PRIO,
        (CPU_STK*)ReadCarCAN_Stk,
        (CPU_STK_SIZE)WATERMARK_STACK_LIMIT/10,
        (CPU_STK_SIZE)TASK_READ_CAR_CAN_STACK_SIZE,
        (OS_MSG_QTY) 0,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    );

}

int main()
{
    OS_ERR err;
    OSInit(&err);
    

    OSTaskCreate( // create fault task
        (OS_TCB *)&FaultState_TCB,
        (CPU_CHAR *)"Fault State",
        (OS_TASK_PTR)&Task_FaultState,
        (void *)NULL,
        (OS_PRIO)1,
        (CPU_STK *)FaultState_Stk,
        (CPU_STK_SIZE)128 / 10,
        (CPU_STK_SIZE)128,
        (OS_MSG_QTY)0,
        (OS_TICK)NULL,
        (void *)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR *)&err);
    assertOSError(OS_MAIN_LOC, err);

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