#include "Tasks.h"
#include "CANbus.h"
#include "BSP_UART.h"
#include "CANConfig.h"

static OS_TCB Task1_TCB;
static CPU_STK Task1_Stk[128];

#define STACK_SIZE 128

#define CARCAN_FILTER_SIZE (sizeof carCANFilterList / sizeof(CANId_t))

#define TEST_FilterListLen 8
const CANId_t TEST_FilterList[TEST_FilterListLen] = {
    MC_PHASE_CURRENT,
    VOLTAGE_VEC,
    CURRENT_VEC,
    BACKEMF,
    TEMPERATURE,
    ODOMETER_AMPHOURS,
    ARRAY_CONTACTOR_STATE_CHANGE,
    CARDATA
};

void Task1(void *p_arg){
    CPU_Init();
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U) OSCfg_TickRate_Hz);
    CANbus_Init(CARCAN, TEST_FilterList, TEST_FilterListLen);
    // CANbus_Init(CARCAN, NULL, 0);

    CANDATA_t dataBuf; // A buffer in which we can store the messages we read

    while(1){

        ErrorStatus status = CANbus_Read(&dataBuf, true, CARCAN);
        if (status != SUCCESS) {
            printf("CANbus Read error status: %d\n\r", status);
        }

        // Print the message ID we receive out
        printf("%d\n\r", dataBuf.ID); 

      
    }

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