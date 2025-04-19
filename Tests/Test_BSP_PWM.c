#include "bsp.h"
#include "StatusLeds.h"

int main(){
    BSP_PWM_Init(69420, 1, true);

    Status_Leds_Init();
    Status_Leds_Toggle(CONTROLS_FAULT_LED);

    BSP_PWM_Set_State(INDICATOR_PWM, true);
    BSP_PWM_Set_State(BRAKE_PWM, true);

    int duty = 2;
    int add = 1;
    volatile int x = 0;
    while(1) {
        x++;
        if(x > 42069){
            x = 0;
            BSP_PWM_Set_Duty_Cycle(duty);
            duty += add;
            if(duty == 25) add = -1;
            else if(duty == 1) add = 1;
        }
    }

    
    Status_Leds_Toggle(CONTROLS_FAULT_LED);
    
    BSP_PWM_Set_State(INDICATOR_PWM, false);
    BSP_PWM_Set_State(BRAKE_PWM, false);
}