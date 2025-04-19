#include "bsp.h"

int main(){
    BSP_PWM_Init(25000, 50);
    BSP_UART_Init(USB);
    printf("=========Commencing PWM Test=======");

    printf("Testing 50%% brightness");
    BSP_PWM_Set_State(INDICATOR_PWM, true);
    BSP_PWM_Set_State(BRAKE_PWM, true);

    volatile int b = 0;
    for(int i = 0; i < 30000; ++i) {b++;}

    printf("Testing off");
    BSP_PWM_Set_State(INDICATOR_PWM, false);
    BSP_PWM_Set_State(BRAKE_PWM, false);
}