#include "Tasks.h"
#include "Contactors.h"
#include "CANbus.h"
#include "CANConfig.h"
#include "StatusLeds.h"

static OS_TCB Task1_TCB;
#define STACK_SIZE 128
static CPU_STK Task1_Stk[STACK_SIZE];

OS_TCB Task2_TCB;
static CPU_STK Task2_Stk[STACK_SIZE];

// Run test in Car CAN loopback mode

void Task_Fake_Contactor_Driver(){

    CPU_Init();
    OS_ERR err;
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U) OSCfg_TickRate_Hz);
    CANDATA_t msg;  
    msg.idx = 0;
    msg.ID = CONTACTOR_SENSE;
    memset(&msg.data, 0x00, sizeof msg.data);
    while(1){
        Status_Leds_Toggle(CRUISE_IND_LED);
        msg.data[0] = 0x01;
        msg.data[1] = 0x00;
        // send fake state of Canbus
        CANbus_Send(msg, true, CARCAN);
        OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_HMSM_STRICT, &err);

    }
}


void Task_Test_Contactors(){

    CPU_Init();
    OS_ERR err;

    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U) OSCfg_TickRate_Hz);
    OSTimeDlyHMSM(0, 0, 5, 0, OS_OPT_TIME_HMSM_STRICT, &err);
    CANDATA_t recv;  
    recv.idx = 0;
    recv.ID = CONTACTOR_SENSE;
    memset(&recv.data, 0x00, sizeof recv.data);
    while(1){
        ErrorStatus stat = Contactors_Set(MOTOR_CONTROLLER_CONTACTOR, ON, true);
        Status_Leds_Write(MOTOR_CONTROLLER_FAULT_LED, stat == SUCCESS ? ON : OFF);
        OSTimeDlyHMSM(0, 0, 5, 0, OS_OPT_TIME_HMSM_STRICT, &err);
        bool set = Contactors_Get(MOTOR_CONTROLLER_CONTACTOR);
        Status_Leds_Write(BPS_FAULT_LED, set == ON ? ON : OFF);
        CANbus_Read(&recv, true, CARCAN);
        Status_Leds_Write(OS_FAULT_LED, ON);
    }
}

int main(){
    Status_Leds_Init();
    Contactors_Init();
    CANbus_Init(CARCAN, (CANId_t *) carCANFilterList, NUM_CARCAN_FILTERS);
    OS_ERR err;
    OSInit(&err);
    assertOSError(err);

    OSTaskCreate(
        (OS_TCB*)&Task1_TCB,
        (CPU_CHAR*)"Task_Test_Contactors",
        (OS_TASK_PTR)Task_Test_Contactors,
        (void*)NULL,
        (OS_PRIO)2,
        (CPU_STK*)Task1_Stk,
        (CPU_STK_SIZE)STACK_SIZE/10,
        (CPU_STK_SIZE)STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR|OS_OPT_TASK_STK_CHK),
        (OS_ERR*)&err
    );
    assertOSError(err);

    OSTaskCreate(
        (OS_TCB*)&Task2_TCB,
        (CPU_CHAR*)"Task_Test_Contactors",
        (OS_TASK_PTR)Task_Fake_Contactor_Driver,
        (void*)NULL,
        (OS_PRIO)2,
        (CPU_STK*)Task2_Stk,
        (CPU_STK_SIZE)STACK_SIZE/10,
        (CPU_STK_SIZE)STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR|OS_OPT_TASK_STK_CHK),
        (OS_ERR*)&err
    );
    assertOSError(err);
    
    OSStart(&err);
    assertOSError(err);
}