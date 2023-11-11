#include "Tasks.h"
#include "common.h"
#include "CANbus.h"
#include "config.h"

static OS_TCB Task1_TCB;
static CPU_STK Task1_Stk[DEFAULT_STACK_SIZE];

void Task1(){
    CPU_Init();
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U) OSCfg_TickRate_Hz);
    BSP_CAN_Init(CAN_1, NULL, NULL, NULL, 0);
}

int main(){
    OS_ERR err;
    OSInit(&err);
    
    if(err != OS_ERR_NONE){
        printf("OS error code %d\n", err);
    }

     OSTaskCreate(
        (OS_TCB*)&Task1_TCB,
        (CPU_CHAR*)"Task1",
        (OS_TASK_PTR)Task1,
        (void*)NULL,
        (OS_PRIO)4,
        (CPU_STK*)Task1_Stk,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE/10,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR|OS_OPT_TASK_STK_CHK),
        (OS_ERR*)&err
     );
     assertOSError(OS_MAIN_LOC, err);

     OSStart(&err);
     assertOSError(OS_MAIN_LOC, err);

     while(1){};
}
