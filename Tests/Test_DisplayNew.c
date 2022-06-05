#include "common.h"
#include "config.h"
#include "os.h"
#include "Tasks.h"
#include "Display.h"
#include "bsp.h"
#include "MotorController.h"

static OS_TCB Task1TCB;
static CPU_STK Task1Stk[DEFAULT_STACK_SIZE];


void Task1(void* arg){
    CPU_Init();
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U) OSCfg_TickRate_Hz);
    SupplementalVoltage = 200;
    Display_Init();

    OS_ERR err;
    Display_SetMainView();
    while(1){
        OSTimeDlyHMSM(0,0,5,0,OS_OPT_TIME_HMSM_STRICT,&err);
        assertOSError(OS_MAIN_LOC,err);
        SupplementalVoltage +=100;
        Display_SetSBPV(SupplementalVoltage);
        Display_SetVelocity(25.0);
    }
};

int main(){
    OS_ERR err;
    OSInit(&err);
    OSSemCreate( //create fault sem4
        &FaultState_Sem4,
        "Fault State Semaphore",
        0,
        &err
    );
    assertOSError(OS_MAIN_LOC,err); 

    OSTaskCreate( //create fault task
        (OS_TCB*)&FaultState_TCB,
        (CPU_CHAR*)"Fault State",
        (OS_TASK_PTR)&Task_FaultState,
        (void*)NULL,
        (OS_PRIO)1,
        (CPU_STK*)FaultState_Stk,
        (CPU_STK_SIZE)128/10,
        (CPU_STK_SIZE)128,
        (OS_MSG_QTY)0,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    );
    assertOSError(OS_MAIN_LOC,err);

    //create tester thread
     OSTaskCreate(
        (OS_TCB*)&Task1TCB,
        (CPU_CHAR*)"Task 1",
        (OS_TASK_PTR)Task1,
        (void*)NULL,
        (OS_PRIO)13,
        (CPU_STK*)Task1Stk,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE/10,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    );
    assertOSError(OS_MAIN_LOC,err);


    OSStart(&err);
} 