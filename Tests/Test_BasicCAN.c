#include "Tasks.h"
#include "CANbus.h"
#include "CANConfig.h"

OS_TCB Task1_TCB;
static CPU_STK Task1_Stk[DEFAULT_STACK_SIZE];

/*
 * Run this test with MotorCAN in loopback mode!
 */

void Task1(void *p_arg) {
    CPU_Init();
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U) OSCfg_TickRate_Hz);
    CANbus_Init(CARCAN, carCANFilterList, sizeof carCANFilterList);
    BSP_UART_Init(USB);

    CANDATA_t msg, out;
    msg.ID = VELOCITY;
    msg.idx = 0;
    memset(&msg.data, 0xa5, sizeof msg.data);

    while (1) {
        CANbus_Send(msg, true, CARCAN);
        CANbus_Read(&out, true, CARCAN);
        printf("Read stuff\n\r");
    }
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
    assertOSError(err);

    OSStart(&err);
    assertOSError(err);
}
