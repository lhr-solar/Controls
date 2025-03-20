#include "Tasks.h"
#include "CANbus.h"
#include "CANConfig.h"

OS_TCB Task1_TCB;
static CPU_STK Task1_Stk[DEFAULT_STACK_SIZE];
bool toggle = false;
/*
 * Run this test with MotorCAN in loopback mode!
 */

void Task1(void *p_arg) {
    CPU_Init();
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U) OSCfg_TickRate_Hz);
    CANbus_Init(CARCAN, carCANFilterList, sizeof carCANFilterList);
    BSP_UART_Init(UART_2);
    BSP_GPIO_Init(PORTC, GPIO_Pin_13, OUTPUT, false);

    // CANDATA_t carMsg = {
    //     .ID=BPS_TRIP,
    //     .idx=0,
    //     .data={0xC, 0xA, 0xF, 0xE, 0xB, 0xA, 0xB, 0xE },
    // };
    

    while (1) {

        volatile uint8_t data̠byte = /*0xFF;*/toggle ? 0xFF : 0x00;
        CANDATA_t carMsg = {
            .ID = BPS_TRIP,
            .idx = 0,
            .data = {data̠byte,data̠byte,data̠byte,data̠byte,data̠byte,data̠byte,data̠byte,data̠byte}
        };

        CANbus_Send(carMsg, false, CARCAN);
        BSP_GPIO_Write_Pin(PORTC, GPIO_Pin_13, toggle);
        for(int i = 0; i < 999999; i++){
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
