/**
 * This is meant to test exception handling in the ReadCarCAN module by pretending to be BPS.
 * 
 * This testfile is intended to be compiled and run on Renode as the supplementary machine to send CAN messages to the main machine.
 * It is designed to send messages on CAN1 (CARCAN) to test ReadCarCAN and FaultState's fault handling
 * Specifically, it tests the exceptions for disabling charging and handling the BPS trip message
 * 
 * Run this test in conjuction with Test_FaultThread_Exceptions
 * 
 * @file
 * @author Madeleine Lee (madeleinercflee@utexas.edu)
 * @brief Supplemental testfile for testing the fault state exception mechanism
 * @version
 * @date 2023-6-08
 * 
 * @copyright Copyright (c) 2022 Longhorn Racing Solar
 * 
*/


#include "Tasks.h"
#include "CANbus.h"
#include "ReadTritium.h"


#define STACK_SIZE 128
#define READY_MSG 0x4444

static OS_TCB Task1_TCB;
static CPU_STK Task1_Stk[STACK_SIZE];




void Task1(void *p_arg){
    CPU_Init();
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U) OSCfg_TickRate_Hz);
    printf("\n\rIn Task1");
    OS_ERR err;
    CANDATA_t dataBuf = {0};
    CANbus_Init(MOTORCAN, NULL, 0);
    uint16_t data = 0;

    CANDATA_t canError = {0};
    canError.ID=MOTOR_STATUS;


    for (int i = 0; i < NUM_TRITIUM_ERRORS + 2; i++) {
         do {
             ErrorStatus status = CANbus_Read(&dataBuf, false, MOTORCAN);
             if (status != SUCCESS) {
                 printf("CANbus Read error status: %d\n\r", status);
             } else {
                 data = dataBuf.data[0];
                 printf("\n\rWaiting for start. Read: %d", data);
             }
             OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &err);
         } while (data != READY_MSG);

        OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_HMSM_STRICT, &err);
        uint16_t tritiumError = (0x01<<i)>>1;
        *((uint16_t*)(&canError.data[4])) = tritiumError;
        printf("\n\rNow sending: %d", tritiumError);

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


  
