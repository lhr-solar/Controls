#include "Tasks.h"
#include "Contactors.h"

static OS_TCB Task1_TCB;
#define STACK_SIZE 128
static CPU_STK Task1_Stk[STACK_SIZE];


/*
 * When running this test on the motor testbench, hardcode the SendTritium task
 * to always send an unobtainable velocity. This ensures that no regen braking
 * takes place
 */

void Task1(){
    OS_ERR err;

    CPU_Init();
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U) OSCfg_TickRate_Hz);
    BSP_UART_Init(UART_2);
    Contactors_Init();

    for (;;) {
        bool set = Contactors_Get(ARRAY_CONTACTOR);
        printf("Turning contactor %s\r\n", set ? "off" : "on");
        Contactors_Set(ARRAY_CONTACTOR, !set, true);
        OSTimeDlyHMSM(0, 0, 5, 0, OS_OPT_TIME_HMSM_STRICT, &err);
    }
}

int main(){
    OS_ERR err;
    OSInit(&err);
    assertOSError(OS_MAIN_LOC, err);

    OSTaskCreate(
        (OS_TCB*)&Task1_TCB,
        (CPU_CHAR*)"Task1",
        (OS_TASK_PTR)Task1,
        (void*)NULL,
        (OS_PRIO)2,
        (CPU_STK*)Task1_Stk,
        (CPU_STK_SIZE)STACK_SIZE/10,
        (CPU_STK_SIZE)STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR|OS_OPT_TASK_STK_CHK),
        (OS_ERR*)&err
    );
    assertOSError(OS_MAIN_LOC, err);
    
    OSStart(&err);
    assertOSError(OS_MAIN_LOC, err);
}