#include "common.h"
#include "config.h"
#include "os.h"
#include "Tasks.h"
#include "bsp.h"
static OS_TCB Task1TCB;
static CPU_STK Task1Stk[DEFAULT_STACK_SIZE];


void Task1(void* arg){
    CPU_Init();
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U) OSCfg_TickRate_Hz);
    OS_ERR err;
    OS_MUTEX testMut;
    OS_TICK tick = (OS_TICK)0;
    CPU_TS ts;
    OSMutexPend(&testMut,tick, OS_OPT_POST_NONE, &ts,&err);
    assertOSError(err);

};

int main(){
    OS_ERR err;
    OSInit(&err);

    //create tester thread
     OSTaskCreate(
        (OS_TCB*)&Task1TCB,
        (CPU_CHAR*)"Task 1",
        (OS_TASK_PTR)Task1,
        (void*)NULL,
        (OS_PRIO)13,
        (CPU_STK*)Task1Stk,
        (CPU_STK_SIZE)128/10,
        (CPU_STK_SIZE)128,
        (OS_MSG_QTY)0,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    );
    assertOSError(err);

    OSStart(&err);
} 