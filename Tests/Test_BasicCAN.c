#include "Tasks.h"
#include "CANbus.h"
#include "CANConfig.h"
#include "daybreak_pins.h"

OS_TCB Task1_TCB;
static CPU_STK Task1_Stk[DEFAULT_STACK_SIZE];

/*
 * Run this test with CarCAN in loopback mode!
 */

void Task1(void *p_arg) {
    CPU_Init();
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U) OSCfg_TickRate_Hz);

    BSP_GPIO_Init(OS_FAULT_PORT, OS_FAULT, OUTPUT, false); // use OS_FAULT pin as debug pin
    BSP_GPIO_Init(BPS_FAULT_PORT, BPS_FAULT, OUTPUT, false); 
    BSP_GPIO_Init(MOTOR_CTRL_FAULT_PORT, MOTOR_CTRL_FAULT, OUTPUT, false);
    BSP_GPIO_Init(CONTROLS_FAULT_PORT, CONTROLS_FAULT, OUTPUT, false);
    

    CANbus_Init(CARCAN, NULL, 0);

    CANDATA_t msg, out;
    msg.ID = BPS_TRIP;
    msg.idx = 0;
    memset(&msg.data, 0xa5, sizeof msg.data);
    BSP_GPIO_Write_Pin(OS_FAULT_PORT, OS_FAULT, ON);

    while (1) {
        ErrorStatus sendError = CANbus_Send(msg, true, CARCAN);
        BSP_GPIO_Write_Pin(CONTROLS_FAULT_PORT, CONTROLS_FAULT, sendError == SUCCESS ? ON : OFF);
        ErrorStatus readError = CANbus_Read(&out, true, CARCAN);
        BSP_GPIO_Write_Pin(BPS_FAULT_PORT, BPS_FAULT, readError == SUCCESS ? ON : OFF);
        BSP_GPIO_Write_Pin(MOTOR_CTRL_FAULT_PORT, MOTOR_CTRL_FAULT, ON);
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
