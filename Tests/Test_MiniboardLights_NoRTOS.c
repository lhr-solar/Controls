#include "common.h"
#include "config.h"
#include <unistd.h>
//#include "BSP_PLL.h"
#include "Minions.h"
#include "BSP_UART.h"

// Test file that sends out the status of FWD_SW to UART_2 without RTOS


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
    Minions_Init();
    while(1){
        for(int i = A_CNCTR;i < RSVD_LED;i++){
            Lights_Toggle(i);
        }

        printf("CRUZ_ST Status: %d\n", Lights_Read(A_CNCTR));
        printf("CRUZ_EN Status: %d\n", Lights_Read(M_CNCTR));
        printf("REV_SW Status: %d\n", Lights_Read(CTRL_FAULT));
        printf("FWD_SW Status: %d\n", Lights_Read(LEFT_BLINK));
        printf("HEADLIGHT_SW Status: %d\n", Lights_Read(RIGHT_BLINK));
        printf("LEFT_SW Status: %d\n", Lights_Read(Headlight_ON));
        printf("RIGHT_SW Status: %d\n", Lights_Read(BrakeLight));
        //TODO: Need to refresh the console
        //delay_u(250);
    }