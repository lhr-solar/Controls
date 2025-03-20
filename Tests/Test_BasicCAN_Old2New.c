#include "Tasks.h"
#include "CANbus.h"
#include "CANConfig.h"
#include "daybreak_pins.h"

OS_TCB Task1_TCB;
static CPU_STK Task1_Stk[DEFAULT_STACK_SIZE];

bool toggle = true;

void Task1(void *p_arg) {
    CPU_Init();
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U) OSCfg_TickRate_Hz);

    BSP_GPIO_Init(OS_FAULT_PORT, OS_FAULT, OUTPUT, false); // use OS_FAULT pin as debug pin
    BSP_GPIO_Init(BPS_FAULT_PORT, BPS_FAULT, OUTPUT, false); 
    BSP_GPIO_Init(MOTOR_CTRL_FAULT_PORT, MOTOR_CTRL_FAULT, OUTPUT, false);
    BSP_GPIO_Init(CONTROLS_FAULT_PORT, CONTROLS_FAULT, OUTPUT, false);
    BSP_GPIO_Init(HEARTBEAT_PORT, HEARTBEAT, OUTPUT, false);
    BSP_GPIO_Init(IG1_PORT, IG1, OUTPUT, false);



    CANbus_Init(CARCAN, (CANId_t *) carCANFilterList, NUM_CARCAN_FILTERS);
    CANbus_Init(MOTORCAN, (CANId_t *) motorCANFilterList, NUM_MOTORCAN_FILTERS);

    CANDATA_t out;
    BSP_GPIO_Write_Pin(OS_FAULT_PORT, OS_FAULT, ON);

    while (1) {
       volatile bool consensus = false;
       for (int i = 0; i < 8; i++) {consensus |= out.data[i];}
      
        ErrorStatus readError = CANbus_Read(&out, false, CARCAN);
        BSP_GPIO_Write_Pin(IG1_PORT, IG1,toggle);
        BSP_GPIO_Write_Pin(HEARTBEAT_PORT, HEARTBEAT, toggle);
        if(out.ID == BPS_TRIP&&
            consensus) {
            BSP_GPIO_Write_Pin(BPS_FAULT_PORT, BPS_FAULT, ON);
            BSP_GPIO_Write_Pin(MOTOR_CTRL_FAULT_PORT, MOTOR_CTRL_FAULT, OFF);
        }else{
            BSP_GPIO_Write_Pin(MOTOR_CTRL_FAULT_PORT, MOTOR_CTRL_FAULT, ON);
        }
        
            for(int i = 0; i < 999999;i++){
            }
            toggle = !toggle;
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