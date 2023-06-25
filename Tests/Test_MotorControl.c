#include "Tasks.h"
#include "CANbus.h"
#include "CAN_Queue.h"
#include "ReadTritium.h"
#include "Display.h"
#include "Pedals.h"
#include "Minions.h"
#include "UpdateDisplay.h"
#include "CANConfig.h"

static OS_TCB Task1_TCB;
#define STACK_SIZE 256
static CPU_STK Task1_Stk[STACK_SIZE];

void p_float(float f) {
    bool neg = f < 0;
    if (neg) f = -f;
    int n = (int) f;
    f -= n;
    printf("%s%d.%d\n\r", neg ? "-" : "", n, (int) (f*100));
}

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
    Pedals_Init();
    Minion_Init();
    Display_Init();
    UpdateDisplay_Init();
    CANbus_Init(MOTORCAN, motorCANFilterList, sizeof motorCANFilterList / sizeof(CANId_t));
    for (;;) {
        printf("Motor velocity is currently ");
        p_float(Motor_Velocity_Get());
        OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_HMSM_STRICT, &err);
        assertOSError(OS_MAIN_LOC, err);
    }
}

int main(){
    OS_ERR err;
    OSInit(&err);
    assertOSError(OS_MAIN_LOC, err);

    OSSemCreate(&FaultState_Sem4, "Fault State Semaphore", 0, &err);
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

    OSTaskCreate(
        (OS_TCB*)&UpdateDisplay_TCB,
        (CPU_CHAR*)"Update Display",
        (OS_TASK_PTR)Task_UpdateDisplay,
        (void*)NULL,
        (OS_PRIO)TASK_UPDATE_DISPLAY_PRIO,
        (CPU_STK*)UpdateDisplay_Stk,
        (CPU_STK_SIZE)STACK_SIZE/10,
        (CPU_STK_SIZE)STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR|OS_OPT_TASK_STK_CHK),
        (OS_ERR*)&err
    );
    assertOSError(OS_MAIN_LOC, err);

    OSTaskCreate(
        (OS_TCB*)&FaultState_TCB,
        (CPU_CHAR*)"Fault State",
        (OS_TASK_PTR)Task_FaultState,
        (void*)NULL,
        (OS_PRIO)TASK_FAULT_STATE_PRIO,
        (CPU_STK*)FaultState_Stk,
        (CPU_STK_SIZE)STACK_SIZE/10,
        (CPU_STK_SIZE)STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR|OS_OPT_TASK_STK_CHK),
        (OS_ERR*)&err
    );
    assertOSError(OS_MAIN_LOC, err);

    OSTaskCreate(
        (OS_TCB*)&SendTritium_TCB,
        (CPU_CHAR*)"Send Tritium",
        (OS_TASK_PTR)Task_SendTritium,
        (void*)NULL,
        (OS_PRIO)TASK_SEND_TRITIUM_PRIO,
        (CPU_STK*)SendTritium_Stk,
        (CPU_STK_SIZE)STACK_SIZE/10,
        (CPU_STK_SIZE)STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR|OS_OPT_TASK_STK_CHK),
        (OS_ERR*)&err
    );
    assertOSError(OS_MAIN_LOC, err);

    OSTaskCreate(
        (OS_TCB*)&ReadTritium_TCB,
        (CPU_CHAR*)"Read Tritium",
        (OS_TASK_PTR)Task_ReadTritium,
        (void*)NULL,
        (OS_PRIO)TASK_READ_TRITIUM_PRIO,
        (CPU_STK*)ReadTritium_Stk,
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