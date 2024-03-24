/**
 * Test for Contactors
 * 
 * When running this test on the motor testbench, hardcode the SendTritium task
 * to always send an unobtainable velocity. This ensures that no regen braking
 * takes place
*/

#include "Tasks.h"
#include "Contactors.h"

#define STACK_SIZE 128
static CPU_STK Task1_Stk[STACK_SIZE];
static OS_TCB Task1_TCB;

void Task1(void) {

    CPU_Init();
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U) OSCfg_TickRate_Hz);

    BSP_UART_Init(UART_2);
    Contactors_Init();

    printf("We did a thing\n\r");

    while(1) {
        bool set = Contactors_Get(ARRAY_PRECHARGE_BYPASS_CONTACTOR);
        printf("Turning contactors %s\r\n", set ? "off" : "on");
        Contactors_Set(ARRAY_PRECHARGE_BYPASS_CONTACTOR, !set, true);
        Contactors_Set(MOTOR_CONTROLLER_PRECHARGE_BYPASS_CONTACTOR, !set, true);

        //for(int i = 0; i < 999999; i++);
    }
}

int main(void) {
    OS_ERR err = 0;

    OSInit(&err);
    assertOSError(err);

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
    assertOSError(err);
    
    OSStart(&err);
    assertOSError(err);
}