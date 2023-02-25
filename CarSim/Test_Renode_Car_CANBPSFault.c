/**
 * This is meant to test the ReadCarCAN module in the controls system, by "pretending" to be BPS.
 * 
 * This testfile is intended to be compiled and run on Renode as the supplementary machine to send CAN messages to the main machine.
 * It is designed to send messages on CAN1 (CARCAN) to test ReadCarCAN's fault handling
 * Specifically, it sends the BPS trip message so we can check that this message sends the car to Fault State
 * 
 * Author: (indirectly) Sidharth N. Babu
 * Date Written: 2023-02-24
 * Email: sidharth.n.babu@utexas.edu
*/


#include "Tasks.h"
#include "CANbus.h"

static OS_TCB Task1_TCB;
static CPU_STK Task1_Stk[128];
#define STACK_SIZE 128



void Task1(void *p_arg){
    CPU_Init();
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U) OSCfg_TickRate_Hz);
    OS_ERR err;
    CANbus_Init(CARCAN, NULL, 0);



    //send enable messages on a loop on time

    // Message for charge enable
    CANDATA_t chargeMsg;
    chargeMsg.ID = CHARGE_ENABLE;
    *(uint64_t*)(&chargeMsg.data) = 0;
    chargeMsg.idx = 0;

    // Message for BPS Fault
    CANDATA_t tripBPSMsg;
    tripBPSMsg.ID = BPS_TRIP;
    *(uint64_t*)(&chargeMsg.data) = 0;
    tripBPSMsg.idx = 0;


    while(1){
        chargeMsg.data[0] = true;
        for(int i = 0; i<10; i++){
            CANbus_Send(chargeMsg, CAN_BLOCKING,CARCAN);
            OSTimeDlyHMSM(0, 0, 0, 400, OS_OPT_TIME_HMSM_STRICT, &err);
        }

        CANbus_Send(tripBPSMsg, CAN_BLOCKING, CARCAN);
        

        for(int i = 0; i<10; i++){
            CANbus_Send(chargeMsg, CAN_BLOCKING,CARCAN);
            OSTimeDlyHMSM(0, 0, 0, 400, OS_OPT_TIME_HMSM_STRICT, &err);
        }
        
        *(uint64_t*)(&chargeMsg.data) = 1;
        CANbus_Send(tripBPSMsg, CAN_BLOCKING, CARCAN);
        
    }

}

int main(void){ //startup OS stuff, spawn test task
    
    OS_ERR err;
    OSInit(&err);
    if(err != OS_ERR_NONE){
        printf("OS error code %d\n",err);
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
        printf("Task error code %d\n", err);
    }
    OSStart(&err);
    if (err != OS_ERR_NONE) {
        printf("OS error code %d\n", err);
    }
    return 0;
}


 


  
