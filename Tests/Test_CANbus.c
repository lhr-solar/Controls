#include "common.h"
#include "os.h"
#include "CANbus.h"
#include <string.h>

static OS_TCB Task1TCB;
static CPU_STK Task1Stk[128];

void createDummyMsg(CANDATA_t *msg, CANId_t id, uint64_t data) {
    msg->ID = id;
    msg->idx = 0;
    memcpy(&msg->data, &data , sizeof data);
}

void Task1(void *p_arg) {
    (void) p_arg;
    OS_ERR err;

    CPU_Init();
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U) OSCfg_TickRate_Hz);

    /**
     * +-------------+
     * | CANbus_Send |
     * +-------------+
    */

    CANDATA_t msg;
    CANDATA_t response;
    msg.ID = MOTOR_DRIVE;
    msg.idx = 0;
    float vel = 312.0f;
    float current = 0.8f;
    memcpy(&msg.data[0], &vel, sizeof vel);
    memcpy(&msg.data[4], &current, sizeof current);

    CANbus_Send(msg, CAN_BLOCKING, MOTORCAN);
    CANbus_Read(&response, CAN_BLOCKING, MOTORCAN);
    OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_HMSM_STRICT, &err);

    
    msg.ID = ARRAY_CONTACTOR_STATE_CHANGE;
    msg.idx = 0;
    msg.data[0] = 1;
    CANbus_Send(msg, CAN_BLOCKING, CARCAN);
    CANbus_Read(&response, CAN_BLOCKING, CARCAN);
    OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_HMSM_STRICT, &err);

    // Now test non-blocking CAN send
    // The latter ones should return an error
    CANDATA_t msg1, msg2, msg3, msg4;
    createDummyMsg(&msg1, MOTOR_DRIVE, 1L);
    createDummyMsg(&msg2, MOTOR_DRIVE, 2L);
    createDummyMsg(&msg3, MOTOR_DRIVE, 3L);
    createDummyMsg(&msg4, MOTOR_DRIVE, 4L);

    volatile ErrorStatus e;
    e = CANbus_Send(msg1, CAN_NON_BLOCKING, MOTORCAN);
    e = CANbus_Send(msg2, CAN_NON_BLOCKING, MOTORCAN);
    e = CANbus_Send(msg3, CAN_NON_BLOCKING, MOTORCAN); // Potentially an error?
    e = CANbus_Send(msg4, CAN_NON_BLOCKING, MOTORCAN); // Almost definitely an error, check in GDB

    OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &err);

    CANbus_Read(&msg1, CAN_NON_BLOCKING, MOTORCAN);
    CANbus_Read(&msg2, CAN_NON_BLOCKING, MOTORCAN);
    CANbus_Read(&msg3, CAN_NON_BLOCKING, MOTORCAN);
    CANbus_Read(&msg4, CAN_NON_BLOCKING, MOTORCAN);
    
    (void) e;
    
    OSTimeDlyHMSM(0, 0, 2, 0, OS_OPT_TIME_HMSM_STRICT, &err);

    
    // Now do the same test, but blocking this time
    // All of the operations should succeed, so we should see four CAN messages
    e = CANbus_Send(msg1, CAN_BLOCKING, MOTORCAN);
    e = CANbus_Send(msg2, CAN_BLOCKING, MOTORCAN);
    e = CANbus_Send(msg3, CAN_BLOCKING, MOTORCAN);
    e = CANbus_Send(msg4, CAN_BLOCKING, MOTORCAN);
    
    CANbus_Read(&msg1, CAN_BLOCKING, MOTORCAN);
    CANbus_Read(&msg2, CAN_BLOCKING, MOTORCAN);
    CANbus_Read(&msg3, CAN_BLOCKING, MOTORCAN);
    CANbus_Read(&msg4, CAN_BLOCKING, MOTORCAN);


    /**
     * +-------------+
     * | CANbus_Read |
     * +-------------+
     */
    // Might have to put CAN in loopback mode to test this?

    


    while (1) {
        OSTimeDlyHMSM(1, 0, 0, 0, OS_OPT_TIME_HMSM_STRICT, &err);
    }
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