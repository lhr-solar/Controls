#include "Dash.h"

uint8_t getDash(dashPin_t pin){
    uint8_t gear = 0;
    uint8_t ign = 0;
    switch(pin){
        case(BRAKE_LED):
            return BSP_GPIO_Read_Pin(BRAKE_LIGHT_PORT, BRAKE_LIGHT);
            break;

        case(GEAR):
            if(BSP_GPIO_Read_Pin(FORWARD_PORT, FORWARD)) {gear = FWD;}
            else if(BSP_GPIO_Read_Pin(REVERSE_PORT, REVERSE)) {gear = REV;}
            else {gear = NUE;}
            return gear;
            break;

        case(CRUZ_SET):
            return BSP_GPIO_Read_Pin(CRUISE_SET_PORT, CRUISE_SET);
            break;

        case(CRUZ_EN):
            return BSP_GPIO_Read_Pin(CRUISE_ENABLE_PORT, CRUISE_ENABLE);
            break;

        case(IGN):
            if(BSP_GPIO_Read_Pin(IG1_PORT, IG1)){return ign = IGN_1;}
            if(BSP_GPIO_Read_Pin(IG2_PORT, IG2)){return ign = IGN_2;}
            return OFF;
            break;

        default:
        return 0;
            break;
    }
}

void dashInit(){
    BSP_GPIO_Init(BRAKE_LIGHT_PORT, BRAKE_LIGHT, OUTPUT, false);    //BRAKE
    BSP_GPIO_Init(FORWARD_PORT, FORWARD, INPUT, false);             //FWD
    BSP_GPIO_Init(REVERSE_PORT, REVERSE, INPUT, false);             //REV
    BSP_GPIO_Init(CRUISE_SET_PORT, CRUISE_SET, INPUT, false);       //CRUZ_ST
    BSP_GPIO_Init(CRUISE_ENABLE_PORT, CRUISE_ENABLE, INPUT, false); //CRUZ_EN
    BSP_GPIO_Init(IG1_PORT, IG1, INPUT, false); // Array
    BSP_GPIO_Init(IG2_PORT, IG2, INPUT, false); // Motor
}
