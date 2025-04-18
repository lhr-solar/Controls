#include "Tasks.h"
#include "Contactors.h"

static OS_TCB Task1_TCB;
#define STACK_SIZE 128
static CPU_STK Task1_Stk[STACK_SIZE];


void Task1(){
    OS_ERR err;

    CPU_Init();
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U) OSCfg_TickRate_Hz);
    Contactors_Init();
    OSTimeDlyHMSM(0, 0, 10, 0, OS_OPT_TIME_HMSM_STRICT, &err);
    while(1){
        ErrorStatus stat = Contactors_Set(MOTOR_CONTROLLER_CONTACTOR, ON, true);
        Status_Leds_Write(MOTOR_CONTROLLER_FAULT_LED, stat == SUCCESS ? ON : OFF);
        OSTimeDlyHMSM(0, 0, 5, 0, OS_OPT_TIME_HMSM_STRICT, &err);
        bool set = Contactors_Get(ARRAY_CONTACTOR);
        Status_Leds_Write(BPS_FAULT_LED, set == ON ? ON : OFF);
    }
}

int main(){
    Status_Leds_Init();
    Contactors_Init();
    OS_ERR err;
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