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
/**
    Tests ability to read Contactor states over can and via sense pins on the Motor controller
 */

 // Alliases for status LEDs
 #define FAKE_CONTACTOR_TASK_LED CRUISE_IND_LED
 #define TEST_CONTACTOR_TASK_HEARTBEAT OS_FAULT_LED
 #define MOTOR_CONTACTOR_STATUS_LED BPS_FAULT_LED
 #define MOTOR_PRECHARGE_CONTACTOR_LED MOTOR_PRECHARGE_CONTACTOR_LED
 #define ARRAY_PRECHARGE_CONTACTOR_LED ARRAY_PRECHARGE_CONTACTOR_LED
 #define MOTOR_CONTROLLER_FAULT_LED MOTOR_CONTROLLER_FAULT_LED

void Task_Fake_Contactor_Driver(){

    OS_ERR err;
    CANDATA_t msg;  
    msg.idx = 0;
    msg.ID = CONTACTOR_SENSE;
    memset(&msg.data, 0x00, sizeof msg.data);
    while(1){
        Status_Leds_Toggle(FAKE_CONTACTOR_TASK_LED);
        msg.data[0] = 0x09;
        msg.data[1] = 0x00;
        // send fake state of Canbus
        CANbus_Send(msg, true, CARCAN);
        OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_HMSM_STRICT, &err);
    }
}


void Task_Test_Contactors(){

    OS_ERR err;
    OSTimeDlyHMSM(0, 0, 5, 0, OS_OPT_TIME_HMSM_STRICT, &err);
    CANDATA_t recv;  
    recv.idx = 0;
    recv.ID = CONTACTOR_SENSE;
    memset(&recv.data, 0x00, sizeof recv.data);
    Contactors_Set(MOTOR_CONTROLLER_CONTACTOR, ON, true);
    while(1){
        memset(&recv.data, 0x00, sizeof recv.data);
        OSTimeDlyHMSM(0, 0, 5, 0, OS_OPT_TIME_HMSM_STRICT, &err);
        bool set = Contactors_Get(MOTOR_CONTROLLER_CONTACTOR, true);
        Status_Leds_Write(MOTOR_CONTACTOR_STATUS_LED, set == ON ? ON : OFF);
        CANbus_Read(&recv, true, CARCAN);
         // Update Array Precharge sense state
        Contactors_Set(ARRAY_PRECHARGE_BYPASS_CONTACTOR, ARRAY_PRECHARGE_ACTUAL_VALUE(recv.data), true);

        // Update Motor Precharge sense state
        Contactors_Set(MOTOR_CONTROLLER_PRECHARGE_BYPASS_CONTACTOR, MOTOR_PRECHARGE_ACTUAL_VALUE(recv.data), true);

        // clear Contactor status LEDs
        Status_Leds_Write(MOTOR_CONTROLLER_FAULT_LED, OFF);
        Status_Leds_Write(MOTOR_PRECHARGE_CONTACTOR_LED, OFF);
        Status_Leds_Write(ARRAY_PRECHARGE_CONTACTOR_LED, OFF);

        if(Contactors_Get(MOTOR_CONTROLLER_CONTACTOR, true)){
            Status_Leds_Write(MOTOR_CONTROLLER_FAULT_LED, ON);
        }
        if(Contactors_Get(MOTOR_CONTROLLER_PRECHARGE_BYPASS_CONTACTOR, true)){
            Status_Leds_Write(MOTOR_PRECHARGE_CONTACTOR_LED, ON);
        }
        if(Contactors_Get(ARRAY_PRECHARGE_BYPASS_CONTACTOR, true)){
            Status_Leds_Write(ARRAY_PRECHARGE_CONTACTOR_LED, ON);          
        }
        Status_Leds_Write(TEST_CONTACTOR_TASK_HEARTBEAT, ON);
    }
}

int main(){
    Status_Leds_Init();
    Contactors_Init();
    CANbus_Init(CARCAN, (CANId_t *) carCANFilterList, NUM_CARCAN_FILTERS);
    OS_ERR err;
    OSInit(&err);
    assertOSError(err);

    CPU_Init();
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U) OSCfg_TickRate_Hz);

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