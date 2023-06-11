/**
 * XXXXXThis is meant to test the ReadCarCAN module in the controls system, by "pretending" to be BPS.
 * 
 * XXXXXThis testfile is intended to be compiled and run on Renode as the supplementary machine to send CAN messages to the main machine.
 * It is designed to send messages on CAN1 (CARCAN) to test ReadCarCAN's fault handling
 * Specifically, it sends the BPS trip message so we can check that this message sends the car to Fault State
 * 
*/


#include "Tasks.h"
#include "CANbus.h"
#include "ReadTritium.h"

static OS_TCB Task1_TCB;
static CPU_STK Task1_Stk[128];
#define STACK_SIZE 128



void Task1(void *p_arg){
    CPU_Init();
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U) OSCfg_TickRate_Hz);
    OS_ERR err;
    CANDATA_t dataBuf = {0};
    CANbus_Init(MOTORCAN, NULL, 0);
    uint16_t data = 0;

    CANDATA_t canError = {0};
    canError.ID=MOTOR_STATUS;


    for (int i = 0; i < NUM_TRITIUM_ERRORS + 2; i++) {
        do {
            ErrorStatus status = CANbus_Read(&dataBuf, true, MOTORCAN);
            if (status != SUCCESS) {
                printf("CANbus Read error status: %d\n\r", status);
            } else {
                data = dataBuf.data[0];
            }
            OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &err);
        } while (data != 0x4444);

        uint16_t tritiumError = (0x01<<i)>>1;
        *((uint16_t*)(&canError.data[4])) = tritiumError;

         // Send error messages to the leader board
        if (tritiumError == T_HALL_SENSOR_ERR) { // Send the message extra times to allow for motor restart
            CANbus_Send(canError, CAN_BLOCKING, MOTORCAN);
            OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_HMSM_STRICT, &err);
            CANbus_Send(canError, CAN_BLOCKING, MOTORCAN);
            OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_HMSM_STRICT, &err);
        }

        CANbus_Send(canError, CAN_BLOCKING, MOTORCAN);
        
    }



}

int main(void){ //startup OS stuff, spawn test task
    
    OS_ERR err;
    OSInit(&err);
    if(err != OS_ERR_NONE){
        printf("OS error code %d\n",err);
    }
    printf("I don't think this will print. Should move UART Init");
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


 


  
