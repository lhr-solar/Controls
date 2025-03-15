#include "Tasks.h"
#include "CANbus.h"
#include "CANConfig.h"

OS_TCB Task1_TCB;
static CPU_STK Task1_Stk[DEFAULT_STACK_SIZE];

/*
 * Run this test with CarCAN in loopback mode!
 */

void Task1(void *p_arg) {
    CPU_Init();
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U) OSCfg_TickRate_Hz);

    BSP_GPIO_Init(PORTC, GPIO_Pin_13, OUTPUT, true); // use UART2 TP (J25) as IO
    BSP_GPIO_Write_Pin(PORTC, GPIO_Pin_13, OFF);
    // BSP_GPIO_Init(PORTA, GPIO_Pin_3, OUTPUT, false); 
    // BSP_GPIO_Init(PORTA, GPIO_Pin_14, OUTPUT, false); 
    

    CANbus_Init(CARCAN, NULL, 0);
    //CANbus_Init(MOTORCAN, (CANId_t *) motorCANFilterList, NUM_MOTORCAN_FILTERS);
                                
    CANDATA_t msg, out;         
    msg.ID = BPS_TRIP;          
    msg.idx = 0;
    memset(&msg.data, 0xa5, sizeof msg.data);
    // BSP_GPIO_Write_Pin(PORTA, GPIO_Pin_2, OFF);
    // BSP_GPIO_Write_Pin(PORTA, GPIO_Pin_3, OFF);
    // BSP_GPIO_Write_Pin(PORTA, GPIO_Pin_14, OFF);

    while (1) {
        CANbus_Send(msg, true, CARCAN);
        ErrorStatus readError = CANbus_Read(&out, true, CARCAN);
        BSP_GPIO_Write_Pin(RCC_AHB1Periph_GPIOA, GPIO_Pin_3, readError == SUCCESS ? ON : OFF);
        //BSP_GPIO_Write_Pin(RCC_AHB1Periph_GPIOA, GPIO_Pin_3, ON);
        //BSP_GPIO_Write_Pin(RCC_AHB1Periph_GPIOA, GPIO_Pin_14, ON);
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