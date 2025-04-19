#include "bsp.h"

int main(){
    BSP_PWM_Init(25000, 50);
    BSP_PWM_Set_State(INDICATOR_PWM, true);
    BSP_PWM_Set_State(BRAKE_PWM, true);
}