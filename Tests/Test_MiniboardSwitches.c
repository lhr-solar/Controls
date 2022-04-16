/*
    Test file that sends out the status of FOR_SW to UART_2 with RTOS
*/

#include "common.h"
#include "config.h"
#include <unistd.h>
//#include "BSP_PLL.h"
#include "stm32f4xx.h"
#include "Minions.h"
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
    Minions_Init();
    while(1){
        Switches_UpdateStates();

        printf("CRUZ_ST Switch Status: %d\n\r", Switches_Read(CRUZ_ST));
        printf("CRUZ_EN Switch Status: %d\n\r", Switches_Read(CRUZ_EN));
        printf("REV_SW Switch Status: %d\n\r", Switches_Read(REV_SW));
        printf("FOR_SW Switch Status: %d\n\r", Switches_Read(FOR_SW));
        printf("HEADLIGHT Switch Status: %d\n\r", Switches_Read(HEADLIGHT_SW));
        printf("LEFT Switch Status: %d\n\r", Switches_Read(LEFT_SW));
        printf("RIGHT Switch Status: %d\n\r", Switches_Read(RIGHT_SW));
        printf("REGEN Switch Status: %d\n\r", Switches_Read(REGEN_SW));
        printf("HZD Switch Status: %d\n\r", Switches_Read(HZD_SW));
        printf("IGN1 Switch Status: %d\n\r", Switches_Read(IGN_1));
        printf("IGN2 Switch Status: %d\n\r", Switches_Read(IGN_2));
        printf("\n\n\n");
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