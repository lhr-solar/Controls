/*
    Test file that sends out the status of FOR_SW to UART_2 with RTOS
*/

#include "common.h"
#include "config.h"
#include <unistd.h>
//#include "BSP_PLL.h"
#include "stm32f4xx.h"
#include "Switches.h"
#include "BSP_UART.h"

// static void delay_u(uint16_t micro)
// {
//   uint32_t delay = BSP_PLL_GetSystemClock() / 1000000;
// 	for(uint32_t i = 0; i < micro; i++)
// 	{
// 		for(uint32_t j = 0; j < delay; j++);
// 	}
// }

OS_TCB Task1_TCB;
CPU_STK Task1_Stk[256];

void Task1(void *p_arg){
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U) OSCfg_TickRate_Hz);
    
    BSP_UART_Init(UART_2);
    Switches_Init();
    while(1){
        switches_t sw = FOR_SW;
        printf("FORWARD Switch Status: %d", Switches_Read(sw));
        //delay_u(250);
    }
    
}


int main(void) {
    OS_ERR err;
    //BSP_PLL_Init();

    OSInit(&err);

    while(err != OS_ERR_NONE);

    OSTaskCreate(&Task1_TCB,
                "Task 1",
                Task1,
                (void *)0,
                1,
                Task1_Stk,
                16,
                256,
                0,
                0,
                (void *)0,
                OS_OPT_TASK_SAVE_FP | OS_OPT_TASK_STK_CHK,
                &err);
    while(err != OS_ERR_NONE);

    __enable_irq();

    OSStart(&err);

    return 0;
}