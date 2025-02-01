#include "Tasks.h"
#include "CANbus.h"
#include "BSP_UART.h"
#include "CANConfig.h"

static OS_TCB Task1_TCB;
static CPU_STK Task1_Stk[128];

#define STACK_SIZE 128

#define CARCAN_FILTER_SIZE (sizeof carCANFilterList / sizeof(CANId_t))

static void assert(bool);

CANDATA_t dataBuf, resultBuf;
uint64_t data = 0xdeadbeef12345678;

/*
 * NOTE: This test must be run with car CAN in loopback mode
 * TODO: automate this, either with arguments to BSP or #define
 */

void Task1(void *p_arg) {
    (void) p_arg;

    CPU_Init();
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U) OSCfg_TickRate_Hz);

    CANbus_Init(CARCAN, carCANFilterList, NUM_CARCAN_FILTERS);

    dataBuf.ID = CHARGE_ENABLE; // First, send a value that we want to be able to receive
    memcpy(&dataBuf.data, &data, sizeof data);

    assert(CANbus_Send(dataBuf, true, CARCAN) == SUCCESS);
    assert(CANbus_Read(&resultBuf, true, CARCAN) == SUCCESS);
    assert(memcmp(&dataBuf, &resultBuf, sizeof dataBuf) == 0);

    dataBuf.ID = ODOMETER_AMPHOURS; // Now, send a message that we shouldn't receive
    assert(CANbus_Send(dataBuf, true, CARCAN) == SUCCESS);
    assert(CANbus_Read(&resultBuf, false, CARCAN) == ERROR); // check that no can message is read

    // Now send a bunch of messages that should be ignored, followed by those that shouldn't
    for (int i=0; i<10; i++) {
        assert(CANbus_Send(dataBuf, true, CARCAN) == SUCCESS);
    }

    dataBuf.ID = BPS_TRIP;
    assert(CANbus_Send(dataBuf, true, CARCAN) == SUCCESS);
    dataBuf.ID = STATE_OF_CHARGE;
    assert(CANbus_Send(dataBuf, true, CARCAN) == SUCCESS);
    dataBuf.ID = SUPPLEMENTAL_VOLTAGE;
    assert(CANbus_Send(dataBuf, true, CARCAN) == SUCCESS);

    // Make sure that only three messages make it through
    assert(CANbus_Read(&resultBuf, true, CARCAN) == SUCCESS);
    assert(resultBuf.ID == BPS_TRIP);
    assert(CANbus_Read(&resultBuf, true, CARCAN) == SUCCESS);
    assert(resultBuf.ID == STATE_OF_CHARGE);
    assert(CANbus_Read(&resultBuf, true, CARCAN) == SUCCESS);
    assert(resultBuf.ID == SUPPLEMENTAL_VOLTAGE);
    assert(CANbus_Read(&resultBuf, false, CARCAN) == ERROR); // No more messages

    printf("Success!\r\n");
    for (;;);
}

int main(void){ //initialize things and spawn task
    OS_ERR err;
    OSInit(&err);
    if(err != OS_ERR_NONE){
        printf("OS error code %d\n\r",err);
    }

    BSP_UART_Init(UART_2);

    OSTaskCreate(
        (OS_TCB*)&Task1_TCB,
        (CPU_CHAR*)"Task1",
        (OS_TASK_PTR)Task1,
        (void*)NULL,
        (OS_PRIO)4,
        (CPU_STK*)Task1_Stk,
        (CPU_STK_SIZE)STACK_SIZE/10,
        (CPU_STK_SIZE)STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR|OS_OPT_TASK_STK_CHK),
        (OS_ERR*)&err
    );

    
    if (err != OS_ERR_NONE) {
        printf("Task1 error code %d\n\r", err);
    }
    OSStart(&err);
    if (err != OS_ERR_NONE) {
        printf("OS error code %d\n\r", err);
    }
    return 0;

}

static void assert(bool cond) {
    if (!cond) __asm("bkpt");
}
