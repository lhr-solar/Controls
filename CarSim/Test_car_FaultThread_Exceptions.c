


#include "Tasks.h"
#include "CANbus.h"
#include "BSP_UART.h"
#include "CANConfig.h"

static OS_TCB Task1_TCB;
static CPU_STK Task1_Stk[128];

#define STACK_SIZE 128
#define READY_MSG 0x4444



void Task1(void *p_arg){
    CPU_Init();
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U) OSCfg_TickRate_Hz);
    printf("\n\rAbout to init Canbus");
    CANbus_Init(CARCAN, NULL, 0);
    printf("\n\rCanbus init-ed");
    OS_ERR err;

    CANDATA_t dataBuf; // A buffer in which we can store the messages we read
    //uint16_t data = 0;

    //  do {
    //          ErrorStatus status = CANbus_Read(&dataBuf, false, MOTORCAN);
    //          if (status != SUCCESS) {
    //              printf("\n\rCANbus Read error status: %d\n\r", status);
    //          } else {
    //              data = *((uint16_t*)(&dataBuf.data[0]));
    //              printf("\n\rWaiting for start. Read: %d", data);
    //          }
    //          OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &err);
    //      } while (data != READY_MSG);

    CANDATA_t chargeMsg;
    chargeMsg.ID = CHARGE_ENABLE;
    *(uint64_t*)(&chargeMsg.data) = 0;
    chargeMsg.idx = 0;

    // Message for BPS Fault
    CANDATA_t tripBPSMsg;
    tripBPSMsg.ID = BPS_TRIP;
    *(uint64_t*)(&tripBPSMsg.data) = 0;
    tripBPSMsg.idx = 0;

    
    while (1) {

    // Send five charge enable messages so the contactors get flipped on
    chargeMsg.data[0] = true;
    for(int i = 0; i<5; i++){
        CANbus_Send(chargeMsg, CAN_BLOCKING,CARCAN);
        printf("\nSent enable chargeMsg %d", i);
        OSTimeDlyHMSM(0, 0, 0, 400, OS_OPT_TIME_HMSM_STRICT, &err);
    }

    // Send five charge disable messages to test prio-2 disable contactor callback
    // Fault state should turn off contactors but not enter a nonrecoverable fault
    chargeMsg.data[0] = false;
    for(int i = 0; i<5; i++){
        CANbus_Send(chargeMsg, CAN_BLOCKING,CARCAN);
        printf("\nSent disable chargeMsg %d", i);
        OSTimeDlyHMSM(0, 0, 0, 400, OS_OPT_TIME_HMSM_STRICT, &err);
    }

    // Send five more charge enable messages so the contactors get flipped on again
    chargeMsg.data[0] = true;
    for(int i = 0; i<5; i++){
        CANbus_Send(chargeMsg, CAN_BLOCKING,CARCAN);
        printf("\nSent enable chargeMsg %d", i);
        OSTimeDlyHMSM(0, 0, 0, 400, OS_OPT_TIME_HMSM_STRICT, &err);
    }

    // Send a trip message of 1 (trip)
    // Note: trip messages are not usually sent,
    // so any trip message (even 0) should be a concern.
    // Maybe we should handle them differently?
    *(uint64_t*)(&tripBPSMsg.data) = 1;
    CANbus_Send(tripBPSMsg, CAN_BLOCKING, CARCAN);
    printf("\nSent trip message %d", tripBPSMsg.data[0]);

    dataBuf.ID = CARDATA;
    *((uint16_t*)(&dataBuf.data[0])) = READY_MSG;
    CANbus_Send(dataBuf, CAN_NON_BLOCKING, CARCAN);
    }

    while(1){};
    



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


  
