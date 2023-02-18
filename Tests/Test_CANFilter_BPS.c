#include"Tasks.h"
#include "CANbus.h"
#include "BSP_UART.h"


static OS_TCB TaskBPS_TCB;
static CPU_STK TaskBPS_Stk[128];
#define STACK_SIZE 128

// Test CANbus hardware filtering by flooding the bus with messages
void Task_BPS(void *p_arg){
    CPU_Init();
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U) OSCfg_TickRate_Hz);
    OS_ERR err;
    CANbus_Init(CARCAN, NULL, 0);

    // An array consisting of messages we read with random "noise" messages interspersed
    int CANMsgs[] = {
        0x001,
        0x024,
        BPS_TRIP,
        CHARGE_ENABLE,
        STATE_OF_CHARGE,
        SUPPLEMENTAL_VOLTAGE,
        MOTOR_DRIVE,
        0x7F9,
        MOTOR_POWER,
        MOTOR_RESET,
        MOTOR_STATUS,
        MC_BUS,
        VELOCITY,
        MC_PHASE_CURRENT,
        0x423,
        VOLTAGE_VEC,
        CURRENT_VEC,
        BACKEMF,
        TEMPERATURE,
        ODOMETER_AMPHOURS,
        ARRAY_CONTACTOR_STATE_CHANGE,
	    CARDATA,
        0x0F23,
        0xFFFF // Ending sentinel- the task's id index returns to the beginning after finding this
    };

    uint8_t idIndex = 0;

    // Make a CAN message
    CANDATA_t msg;
    msg.ID = 0x001;
    msg.idx = 0; // Do I need to test longer messages?
    *(uint64_t*)(&msg.data) = 0; // Just an empty message without data

    

    while(1){
        
        CANbus_Send(msg, true, CARCAN); // Send the current message and print the id to the uart
        printf("Sent %d\n\r", msg.ID);

        // If we're at the end of the message array, set the index back to 0.
        if (msg.ID == 0xFFFF) {
            idIndex = 0;
        } else { // Otherwise increment the index
            idIndex++;
        }

        msg.ID = CANMsgs[idIndex]; // Set the ID to send to the next message id

        OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &err);
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
        (OS_PRIO)3, // Don't know if this is correct @@@@@
        (CPU_STK*)TaskBPS_Stk,
        (CPU_STK_SIZE)STACK_SIZE,
        (CPU_STK_SIZE)STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR|OS_OPT_TASK_STK_CHK), // Not sure if this is necessary @@@@@
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
