#include "common.h"
#include "config.h"
#include <unistd.h>
//#include "BSP_PLL.h"
#include "Minions.h"
#include "BSP_UART.h"

// Test file that sends out the status of FOR_SW to UART_2 without RTOS


// static void delay_u(uint16_t micro)
// {
//   uint32_t delay = BSP_PLL_GetSystemClock() / 1000000;
// 	for(uint32_t i = 0; i < micro; i++)
// 	{
// 		for(uint32_t j = 0; j < delay; j++);
// 	}
// }

int main(){
    BSP_UART_Init(UART_2);
    Switches_Init();
    switches_t sw = FOR_SW;
    while(1){
        printf("FOWARD Switch Status: %d\n", Switches_Read(sw));
        //delay_u(250);
    }
}