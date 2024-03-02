/**
 * Test file to check that CAN loopback is working properly
 * 
 * Designed to be stepped through with a debugger such as GDB
 * 
 * Run make simulator with proper parameters, setting LOOPBACK 
 * equal to 1 or 0
 * 
 * After changing the parameter, run make clean 
 * to ensure that macro is changed
 * 
 */ 

#include "Tasks.h"
#include "common.h"
#include "CANbus.h"
#include "config.h"
#include "CANConfig.h"

static OS_TCB Task1_TCB;
static CPU_STK Task1_Stk[DEFAULT_STACK_SIZE];

void Task1(){
    CPU_Init();
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U) OSCfg_TickRate_Hz);
    CANbus_Init(CARCAN, (CANId_t*)carCANFilterList, NUM_CARCAN_FILTERS);
    CANbus_Init(MOTORCAN, NULL, NUM_MOTORCAN_FILTERS);
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
     assertOSError(err);

     OSStart(&err);
     assertOSError(err);

     while(1){};
}
