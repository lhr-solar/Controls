#include "Tasks.h"
#include "CANbus.h"
#include "daybreak_pins.h"
#include "StatusLeds.h"
#include "CANConfig.h"


#define TEST_CARCAN_LOOPBACK

  #ifndef TEST_MOTORCAN_lOOPBACK
     #define TEST_CARCAN_LOOPBACK
 #endif

 OS_TCB Task1_TCB;
static CPU_STK Task1_Stk[DEFAULT_STACK_SIZE];

 int task1(){
    Status_Leds_Write(OS_FAULT_LED, true);
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

    while(1){
        ErrorStatus sendError = CANbus_Send(msg, true, bus);
        Status_Leds_Write(CONTROLS_FAULT_LED, sendError == SUCCESS ? ON : OFF);
        ErrorStatus readError = CANbus_Read(&out, true, bus);
        Status_Leds_Write(BPS_FAULT_LED, readError == SUCCESS ? ON : OFF);
        Status_Leds_Write(MOTOR_CONTROLLER_FAULT_LED, ON);
    }
 }

int main(){
    Status_Leds_Init();

    OS_ERR err;
    OSInit(&err);

    TaskSwHook_Init();


    #ifdef TEST_CARCAN_LOOPBACK
    CANbus_Init(CARCAN, (CANId_t *) carCANFilterList, NUM_CARCAN_FILTERS);
    #elif TEST_MOTORCAN_lOOPBACK
    CANbus_Init(MOTORCAN, (CANId_t *) motorCANFilterList, NUM_MOTORCAN_FILTERS);
    #endif

    OSTaskCreate(
        (OS_TCB*)&Task1_TCB,
        (CPU_CHAR*)"Task1",
        (OS_TASK_PTR)task1,
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