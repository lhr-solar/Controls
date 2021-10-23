#include "Tasks.h"
#include "CANbus.h"
#include "stm32f4xx.h"

static OS_TCB Task1_TCB;
static CPU_STK Task1_Stk[128];

void Task1(void *p_arg){
    CPU_Init();
    // OS_CPU_SysTickInit();
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U) OSCfg_TickRate_Hz);
    OS_ERR err;
    CPU_TS ts;
    OSQCreate(&CANBus_MsgQ, "CANBus Q", 16, &err); //initializes CAN Send queue
    
    //spawn can send task
    OSTaskCreate(
        (OS_TCB*)&SendCarCAN_TCB,
        (CPU_CHAR*)"SendCarCan",
        (OS_TASK_PTR)Task_SendCarCAN,
        (void*)NULL,
        (OS_PRIO)3,
        (CPU_STK*)SendCarCAN_Stk,
        (CPU_STK_SIZE)sizeof(SendCarCAN_Stk)/10,
        (CPU_STK_SIZE)sizeof(SendCarCAN_Stk),
        (OS_MSG_QTY)NULL,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR|OS_OPT_TASK_STK_CHK),
        (OS_ERR*)&err
    );

    uint8_t buffer[8];
    buffer[0]=99; //this should change after read
    CANMSG_t msg;
    CANPayload_t payload;
    CANData_t dat;
    dat.b = 1;
    payload.bytes = 1;
    payload.data = dat;
    payload.idx = 0;
    msg.id = CAR_STATE; //ON-OFF 
    msg.payload = payload;
    uint8_t i = 0;
    while(i<4){
        //post a message to the queue
        OSQPost(
            &CANBus_MsgQ,
            &msg,
            sizeof(msg),
            OS_OPT_POST_FIFO,
            &err
        );
        //Test the reading functionality
        // CANbus_Read(&buffer[0],CAN_NON_BLOCKING);
        // printf("Read Result: %i\n",buffer[0]);
        // msg.payload.data.b ^= 0x01; //toggle bit 0  to swap data
        i++;
    }
}
int main(void){ //startup OS stuff, spawn test task
    
    OS_ERR err;
    OSInit(&err);
    if(err != OS_ERR_NONE){
        printf("OS error code %d\n",err);
    }
    OSTaskCreate(
        (OS_TCB*)&Task1_TCB,
        (CPU_CHAR*)"Task1",
        (OS_TASK_PTR)Task1,
        (void*)NULL,
        (OS_PRIO)4,
        (CPU_STK*)Task1_Stk,
        (CPU_STK_SIZE)sizeof(Task1_Stk)/10,
        (CPU_STK_SIZE)sizeof(Task1_Stk),
        (OS_MSG_QTY)NULL,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR|OS_OPT_TASK_STK_CHK),
        (OS_ERR*)&err
    );

    if (err != OS_ERR_NONE) {
        printf("Task1 error code %d\n", err);
    }
    OSStart(&err);
    if (err != OS_ERR_NONE) {
        printf("OS error code %d\n", err);
    }
    return 0;
}

// DEBUG hardfault tracker
// void HardFault_Handler(){
//     while(1){
//         volatile int x = 0;
//     }
// }