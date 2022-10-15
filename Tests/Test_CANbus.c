#include "common.h"
#include "os.h"
#include "CANbus.h"
#include <string.h>

static OS_TCB Task1TCB;
static CPU_STK Task1Stk[128];

void Task1(void *p_arg) {
    OS_ERR err;

    CPU_Init();
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U) OSCfg_TickRate_Hz);

    CANDATA_t msg;
    msg.ID = MOTOR_DRIVE;
    msg.idx = 0;
    float vel = 312.0f;
    float current = 0.8f;
    memcpy(&msg.data[0], &vel, sizeof vel);
    memcpy(&msg.data[4], &current, sizeof current);

    CANbus_Send(msg, CAN_NON_BLOCKING, MOTORCAN);
    OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_HMSM_STRICT, &err);

    msg.ID = ARRAY_CONTACTOR_STATE_CHANGE;
    msg.idx = 0;
    msg.data[0] = 1;
    CANbus_Send(msg, CAN_NON_BLOCKING, CARCAN);

}



int main(void) {
    OS_ERR err;
    OSInit(&err);
    if (CANbus_Init(CARCAN) == ERROR) return -1;
    if (CANbus_Init(MOTORCAN) == ERROR) return -1;

    OSTaskCreate(
        &Task1TCB,
        "Task 1",
        Task1,
        NULL,
        4,
        Task1Stk,
        64,
        128,
        0,
        0,
        NULL,
        OS_OPT_TASK_STK_CLR | OS_OPT_TASK_STK_CHK,
        &err
    );

    OSStart(&err);

}