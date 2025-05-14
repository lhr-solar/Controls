#include "Dashboard.h"

// Boolean used to ensure that if car turns on in non-park, it'll be overriden to park 
// until the switch is moved to park; then, it'll follow the specified gear state afterward.
static bool neutralReset = true;

gear_t getGear(void) {
    bool fwdSwitch = BSP_GPIO_Read_Pin(FORWARD_PORT, FORWARD);
    bool revSwitch = BSP_GPIO_Read_Pin(REVERSE_PORT, REVERSE);

    // Check for gear fault
    if(fwdSwitch && revSwitch) {return DASH_GEAR_FAULT_ERROR;}

    // Check for manual override into neutral
    if(neutralReset && !fwdSwitch && !revSwitch) {return DASH_NEU;}

    // Normal check
    neutralReset = false;
    if(fwdSwitch) {return DASH_FWD;}
    else if(revSwitch) {return DASH_REV;}
    else {return DASH_NEU;}
}

switch_state_t getSwitchState(dash_pin_t pin){
    switch(pin){
        case(DASH_CRUZ_SET):
            return BSP_GPIO_Read_Pin(CRUISE_SET_PORT, CRUISE_SET) ? DASH_SWITCH_ON : DASH_SWITCH_OFF;
            break;


        case(DASH_CRUZ_EN):
            return BSP_GPIO_Read_Pin(CRUISE_ENABLE_PORT, CRUISE_ENABLE) ? DASH_SWITCH_ON : DASH_SWITCH_OFF;
            break;


        default:
            return DASH_SWITCH_ERROR;
            break;
    }
}

void dashboardInit(){
    BSP_GPIO_Init(BRAKE_LIGHT_PORT, BRAKE_LIGHT, OUTPUT, false);    //BRAKE
    BSP_GPIO_Init(FORWARD_PORT, FORWARD, INPUT, false);             //FWD
    BSP_GPIO_Init(REVERSE_PORT, REVERSE, INPUT, false);             //REV
    BSP_GPIO_Init(CRUISE_SET_PORT, CRUISE_SET, INPUT, false);       //CRUZ_ST
    BSP_GPIO_Init(CRUISE_ENABLE_PORT, CRUISE_ENABLE, INPUT, false); //CRUZ_EN
    //Dash LEDs initialized in StatusLeds.c
}