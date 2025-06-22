#include "Dashboard.h"
#include "Tasks.h"

// Boolean used to ensure that if car turns on in non-neutral, it'll be overriden to neutral 
// until the switch is moved to neutral; then, it'll follow the specified gear state afterward.

gear_t getGear(bool useOSDelay) {
    static bool neutralReset = true;
    bool fwd;
    bool rev;
    bool neu;

    // Makes sure we're not in a transition (not sufficient to just saw !fwd && !rev => neu)
    neu = true;
    for(int i = 0; i < NEUTRAL_DEBOUNCE_COUNT; i++) {
        fwd = BSP_GPIO_Read_Pin(FORWARD_PORT,  FORWARD);
        rev = BSP_GPIO_Read_Pin(REVERSE_PORT, REVERSE);
        if (fwd && rev) {
            return DASH_GEAR_FAULT_ERROR;
        }
        if(fwd || rev) {
            neu = false;
            break;
        }
        if(useOSDelay == GEAR_USE_OS_DELAY){
            OS_ERR err;
            OSTimeDlyHMSM(0, 0, 0, NEUTRAL_DEBOUNCE_DLY_MS, OS_OPT_TIME_HMSM_STRICT, &err);
            assertOSError(err);
        }
        else{
            delay_ms(NEUTRAL_DEBOUNCE_DLY_MS);
        }
    }

    // Until we've actually seen neutral, manually override to neutral
    if (neutralReset) {
        // only clear neutralReset once we see the switches truly in neutral
        if (neu) {
            neutralReset = false;
        }
        return DASH_NEU;
    }

    // After the initial neutral has been observed, obey the switches normally:
    if (fwd) {
        return DASH_FWD;
    } else if (rev) {
        return DASH_REV;
    } else {
        return DASH_NEU;
    }
}

// NOTE: Uncomment when we're using cruise
switch_state_t getSwitchState(dash_pin_t pin){
    switch(pin){
        case(DASH_CRUZ_SET):
            return BSP_GPIO_Read_Pin(CRUISE_SET_PORT, CRUISE_SET) ? DASH_SW_ON : DASH_SW_OFF;
            break;
        case(DASH_CRUZ_EN):
            return BSP_GPIO_Read_Pin(CRUISE_ENABLE_PORT, CRUISE_ENABLE) ? DASH_SW_ON : DASH_SW_OFF;
            break;
        case(DASH_RIGHT_SIG):
            return BSP_GPIO_Read_Pin(RIGHT_IND_PORT, RIGHT_IND) ? DASH_SW_ON : DASH_SW_OFF;
            break;
        case(DASH_LEFT_SIG):
            return BSP_GPIO_Read_Pin(LEFT_IND_PORT, LEFT_IND) ? DASH_SW_ON : DASH_SW_OFF;
            break;
        default:
            return DASH_SW_ERROR;
            break;
    }
}

void dashboardInit(){
    BSP_GPIO_Init(BRAKE_LIGHT_PORT, BRAKE_LIGHT, OUTPUT, false);    //BRAKE
    BSP_GPIO_Init(FORWARD_PORT, FORWARD, INPUT, false);             //FWD
    BSP_GPIO_Init(REVERSE_PORT, REVERSE, INPUT, false);             //REV
    BSP_GPIO_Init(CRUISE_SET_PORT, CRUISE_SET, INPUT, false);       //CRUZ_ST
    //Dash LEDs initialized in StatusLeds.c
}