#include"Tasks.h"
#include "CANbus.h"


static OS_TCB TaskBPS_TCB;
static CPU_STK TaskBPS_Stk[128];
#define STACK_SIZE 128

//#include bsp UART probably need this @@@@@@@@

// Send a bunch of messages
void Task_BPS(void *p_arg){
    CPU_Init(); // I guess we need to start this?
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U) OSCfg_TickRate_Hz); // I don't know what this is
    OS_ERR err;
    CANbus_Init(CARCAN); // Yes, I think so

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
        0xFFFF // Ending sentinel thing
    };

    uint8_t idIndex = 0;

    // Make a CAN message
    CANDATA_t msg;
    msg.ID = 0x001;
    msg.idx = 0; // Do I need to test longer messages?@@@@
    *(uint64_t*)(&msg.data) = 0; // No data in the messages

    

    while(1){
        //Do any message changes
        CANbus_Send(msg, true, CARCAN); //is blocking ok?
        if (msg.ID == 0xFFFF) {
            idIndex = 0;
        } else {
            idIndex++;
        }

        msg.ID = CANMsgs[idIndex];

        OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &err);
    }

}



int main(void){ //start up OS stuff, spawn test task
    OS_ERR err;
    OSInit(&err);
    if(err != OS_ERR_NONE){
        printf("OS error code %d\n", err);
    }
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
        printf("TaskBPS error code %d\n", err);
    }
    OSStart(&err);
    if (err != OS_ERR_NONE) {
        printf("OS error code %d\n", err);
    }
    return 0;

}
//Spam task:
// Make a CAN message with a random ID and send it 
//(check what IDs to send)

//ReadCarCAN: have it read in things

// Would be nice to print out what gets sent and what is actually taken in
