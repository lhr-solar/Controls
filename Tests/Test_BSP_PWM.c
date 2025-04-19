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
    for(int i = 0; i < 4200000; ++i) {
        x++;
        if(x > 25000){
            x = 0;
            BSP_PWM_Set_Duty_Cycle(duty);
            duty += add;
            if(x == 25) add = -1;
            else if(x == 1) add = 1;
        }
    }

    
    Status_Leds_Toggle(CONTROLS_FAULT_LED);
    
    BSP_PWM_Set_State(INDICATOR_PWM, false);
    BSP_PWM_Set_State(BRAKE_PWM, false);
}