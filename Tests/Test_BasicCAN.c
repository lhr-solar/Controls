#include "Tasks.h"
#include "CANbus.h"
#include "CANConfig.h"
#include "daybreak_pins.h"

OS_TCB Task1_TCB;
static CPU_STK Task1_Stk[DEFAULT_STACK_SIZE];

/**
    Test the CANbus driver in loopback define either TEST_CARCAN_LOOPBACK or TEST_MOTORCAN_LOOPBACK
    Not defining either will default to MOTORCAN loopback
 */

 #define TEST_CARCAN_LOOPBACK
 #ifndef TEST_MOTORCAN_lOOPBACK
    #define TEST_CARCAN_LOOPBACK
#endif


void Task1(void *p_arg) {
    CPU_Init();
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U) OSCfg_TickRate_Hz);

    BSP_GPIO_Init(OS_FAULT_PORT, OS_FAULT, OUTPUT, false); // use OS_FAULT pin as debug pin
    BSP_GPIO_Init(BPS_FAULT_PORT, BPS_FAULT, OUTPUT, false); 
    BSP_GPIO_Init(MOTOR_CTRL_FAULT_PORT, MOTOR_CTRL_FAULT, OUTPUT, false);
    BSP_GPIO_Init(CONTROLS_FAULT_PORT, CONTROLS_FAULT, OUTPUT, false);


    #ifdef TEST_CARCAN_LOOPBACK
    CANbus_Init(CARCAN, (CANId_t *) carCANFilterList, NUM_CARCAN_FILTERS);
    #elif TEST_MOTORCAN_lOOPBACK
    CANbus_Init(MOTORCAN, (CANId_t *) motorCANFilterList, NUM_MOTORCAN_FILTERS);
    #endif

    CANDATA_t msg, out;    
    CAN_t bus;     
    msg.idx = 0;
    memset(&msg.data, 0xa5, sizeof msg.data);


    #ifdef TEST_MOTORCAN_LOOPBACK
    bus = MOTORCAN;
    msg.ID = VELOCITY;         
    #elif defined(TEST_CARCAN_LOOPBACK)
    bus = CARCAN;
    msg.ID = BPS_TRIP;
    #endif

    BSP_GPIO_Write_Pin(OS_FAULT_PORT, OS_FAULT, ON);

    while (1) {
        ErrorStatus sendError = CANbus_Send(msg, true, bus);
        BSP_GPIO_Write_Pin(CONTROLS_FAULT_PORT, CONTROLS_FAULT, sendError == SUCCESS ? ON : OFF);
        ErrorStatus readError = CANbus_Read(&out, true, bus);
        BSP_GPIO_Write_Pin(BPS_FAULT_PORT, BPS_FAULT, readError == SUCCESS ? ON : OFF);
        BSP_GPIO_Write_Pin(MOTOR_CTRL_FAULT_PORT, MOTOR_CTRL_FAULT, ON);
    }
}

int main(){
    OS_ERR err;
    OSInit(&err);

    TaskSwHook_Init();

    OSTaskCreate(
        (OS_TCB*)&Task1_TCB,
        (CPU_CHAR*)"Task1",
        (OS_TASK_PTR)Task1,
        (void*)NULL,
        (OS_PRIO)4,
        (CPU_STK*)Task1_Stk,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE/10,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE,
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

/*
Note: CAN2 start filter bank number n is configurable by writing 
          CAN2SB[5:0] bits in the CAN_FMR register
*/