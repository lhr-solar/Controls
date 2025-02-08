#include "Tasks.h"
#include "CANbus.h"
#include "CANConfig.h"

OS_TCB Task1_TCB;
static CPU_STK Task1_Stk[DEFAULT_STACK_SIZE];

/*
 * Run this test with MotorCAN in loopback mode!
 */

void Task1(void *p_arg) {
    OS_ERR err;

    CPU_Init();
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U) OSCfg_TickRate_Hz);
    CANbus_Init(MOTORCAN, NULL, sizeof motorCANFilterList);
    CANbus_Init(CARCAN, carCANFilterList, sizeof carCANFilterList);
    BSP_UART_Init(USB);
    
    OSTaskCreate(
        (OS_TCB*)&ReadTritium_TCB,
        (CPU_CHAR*)"ReadTritium",
        (OS_TASK_PTR)Task_ReadTritium,
        (void*)NULL,
        (OS_PRIO)TASK_READ_TRITIUM_PRIO,
        (CPU_STK*)ReadTritium_Stk,
        (CPU_STK_SIZE)TASK_READ_TRITIUM_STACK_SIZE/10,
        (CPU_STK_SIZE)TASK_READ_TRITIUM_STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR|OS_OPT_TASK_STK_CHK),
        (OS_ERR*)&err
    );
    assertOSError(OS_MAIN_LOC, err);

    CANDATA_t msg;
    msg.ID = VELOCITY;
    msg.idx = 0;
    memset(&msg.data, 0xa5, sizeof msg.data);

    // Keep ReadTritium alive for 5 seconds
    for (int i=0; i<10; i++) {
        printf("Sending velocity message\n\r");
        CANbus_Send(msg, true, MOTORCAN);
        OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &err);
    }

    printf("Not sending any messages\n\r");

    // Now let ReadTritium hit its timeout until it faults
    OSTaskDel(NULL, &err);
}

int main(){
    OS_ERR err;
    OSInit(&err);

    TaskSwHook_Init();

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
}
