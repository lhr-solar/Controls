#include "Dashboard.h"

switch_state getDashState(dashPin_t pin){
    switch(pin){
        case(BRAKE_LED):
            return BSP_GPIO_Read_Pin(BRAKE_LIGHT_PORT, BRAKE_LIGHT) ? SWITCH_ON : SWITCH_OFF;
            break;

        case(GEAR):
            if(BSP_GPIO_Read_Pin(FORWARD_PORT, FORWARD)) {return FWD;}
            else if(BSP_GPIO_Read_Pin(REVERSE_PORT, REVERSE)) {return REV;}
            else {return NUE;}
            break;

        case(CRUZ_SET):
            return BSP_GPIO_Read_Pin(CRUISE_SET_PORT, CRUISE_SET) ? SWITCH_ON : SWITCH_OFF;
            break;

        case(CRUZ_EN):
            return BSP_GPIO_Read_Pin(CRUISE_ENABLE_PORT, CRUISE_ENABLE) ? SWITCH_ON : SWITCH_OFF;
            break;
        default:
            return SWITCH_ERROR;
            break;
    }
}

void dashboardInit(){
    BSP_GPIO_Init(BRAKE_LIGHT_PORT, BRAKE_LIGHT, OUTPUT, false);    //BRAKE
    BSP_GPIO_Init(FORWARD_PORT, FORWARD, INPUT, false);             //FWD
    BSP_GPIO_Init(REVERSE_PORT, REVERSE, INPUT, false);             //REV
    BSP_GPIO_Init(CRUISE_SET_PORT, CRUISE_SET, INPUT, false);       //CRUZ_ST
    BSP_GPIO_Init(CRUISE_ENABLE_PORT, CRUISE_ENABLE, INPUT, false); //CRUZ_EN
}