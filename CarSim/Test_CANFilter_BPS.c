#include"Tasks.h"
#include "CANbus.h"
#include "BSP_UART.h"
#include "CANConfig.h"


static OS_TCB TaskBPS_TCB;
static CPU_STK TaskBPS_Stk[128];
#define STACK_SIZE 128

#define CARCAN_FILTER_SIZE (sizeof carCANFilterList / sizeof(CANId_t))
#define CANMSG_ARR_SIZE (sizeof(CANMsgs) / sizeof(CANMsgs[0]))

// Test CANbus hardware filtering by flooding the bus with messages
void Task_BPS(void *p_arg){
    CPU_Init();
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U) OSCfg_TickRate_Hz);
    OS_ERR err;
    // CANbus_Init(CARCAN, carCANFilterList, CARCAN_FILTER_SIZE);
    CANbus_Init(CARCAN, carCANFilterList, 0);

    // An array consisting of messages we read with random "noise" messages interspersed
    int CANMsgs[] = {
        BPS_TRIP,
        BPS_CONTACTOR,
        STATE_OF_CHARGE,
        SUPPLEMENTAL_VOLTAGE,
        MOTOR_DRIVE,
        MOTOR_POWER,
        MOTOR_RESET,
        MOTOR_STATUS,
        MC_BUS,
        VELOCITY,
        MC_PHASE_CURRENT,
        VOLTAGE_VEC,
        CURRENT_VEC,
        BACKEMF,
        TEMPERATURE,
        ODOMETER_AMPHOURS,
        ARRAY_CONTACTOR_STATE_CHANGE,
	    CARDATA
    };

    // Make a CAN message. Message id is set in the for loop
    CANDATA_t msg;
    msg.idx = 0;
    *(uint64_t*)(&msg.data) = 0; // Just an empty message without data

    

    while(1){
        
        for (uint8_t i = 0; i < CANMSG_ARR_SIZE; i++) {
            msg.ID = CANMsgs[i];
            CANbus_Send(msg, true, CARCAN); // Send the current message
            printf("Sent %d\n\r", msg.ID); //print the id to the uart
            OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &err);
        }
        
    }

}



int main(void){ //OS initialization and task spawning
    OS_ERR err;
    OSInit(&err);
    if(err != OS_ERR_NONE){
        printf("OS error code %d\n\r", err);
    }

    BSP_UART_Init(UART_2);       

    OSTaskCreate(
        (OS_TCB*)&TaskBPS_TCB,
        (CPU_CHAR*)"TaskBPS",
        (OS_TASK_PTR)Task_BPS,
        (void*)NULL,
        (OS_PRIO)3,
        (CPU_STK*)TaskBPS_Stk,
        (CPU_STK_SIZE)STACK_SIZE,
        (CPU_STK_SIZE)STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR|OS_OPT_TASK_STK_CHK),
        (OS_ERR*)&err
    );

    if (err != OS_ERR_NONE) {
        printf("TaskBPS error code %d\n\r", err);
    }
    OSStart(&err);
    if (err != OS_ERR_NONE) {
        printf("OS error code %d\n\r", err);
    }
    return 0;

}
