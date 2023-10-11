/**
 * @file
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @brief This is meant to test the ReadCarCAN module in the controls system, by
 * "pretending" to be BPS.
 *
 * This testfile is intended to be compiled and run on Renode, as the
 * supplementary machine to send CAN messages to the main machine. It is
 * designed to send messages on CAN1 (CARCAN) and go through all possible states
 * of BPS sending us Charge messages. This includes sending us the message on
 * time, sending us the message late, sending us the message early, and sending
 * us the DISABLE message. It also tests a boundary condition case, where it
 * alternates between DISABLE and ENABLE, testing that the main machine can
 * handle this.
 */

#include "CANbus.h"
#include "Tasks.h"

static OS_TCB Task1_TCB;
static CPU_STK Task1_Stk[128];
#define STACK_SIZE 128

void Task1(void *p_arg) {
    CPU_Init();
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U)OSCfg_TickRate_Hz);
    OS_ERR err;
    CANbus_Init(CARCAN);

    // send enable messages on a loop on time
    CANDATA_t msg;
    msg.ID = CHARGE_ENABLE;
    *(uint64_t *)(&msg.data) = 0;
    msg.idx = 0;

    while (1) {
        msg.data[0] = true;
        for (int i = 0; i < 10; i++) {
            CANbus_Send(msg, CAN_BLOCKING, CARCAN);
            OSTimeDlyHMSM(0, 0, 0, 400, OS_OPT_TIME_HMSM_STRICT, &err);
        }

        // be late by a full cycle
        OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &err);

        // send enable messages on a loop late
        for (int i = 0; i < 10; i++) {
            CANbus_Send(msg, CAN_BLOCKING, CARCAN);
            OSTimeDlyHMSM(0, 0, 0, 600, OS_OPT_TIME_HMSM_STRICT, &err);
        }

        // check boundary condition of alternating enable/disable
        for (int i = 0; i < 10; i++) {
            msg.data[0] = !msg.data[0];
            CANbus_Send(msg, CAN_BLOCKING, CARCAN);
            OSTimeDlyHMSM(0, 0, 0, 400, OS_OPT_TIME_HMSM_STRICT, &err);
        }
    }
}
int main(void) {  // startup OS stuff, spawn test task

    OS_ERR err;
    OSInit(&err);
    if (err != OS_ERR_NONE) {
        printf("OS error code %d\n", err);
    }
    OSTaskCreate((OS_TCB *)&Task1_TCB, (CPU_CHAR *)"Task1", (OS_TASK_PTR)Task1,
                 (void *)NULL, (OS_PRIO)4, (CPU_STK *)Task1_Stk,
                 (CPU_STK_SIZE)STACK_SIZE / 10, (CPU_STK_SIZE)STACK_SIZE,
                 (OS_MSG_QTY)0, (OS_TICK)NULL, (void *)NULL,
                 (OS_OPT)(OS_OPT_TASK_STK_CLR | OS_OPT_TASK_STK_CHK),
                 (OS_ERR *)&err);

    if (err != OS_ERR_NONE) {
        printf("Task1 error code %d\n", err);
    }
    OSStart(&err);
    if (err != OS_ERR_NONE) {
        printf("OS error code %d\n", err);
    }
    return 0;
}
