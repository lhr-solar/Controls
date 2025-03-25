#include "StatusLeds.h"

/**
 * @brief Initializes Status LED pins
 * 
 */
void Status_Leds_Init(void){
    BSP_GPIO_Init(OS_FAULT_PORT, OS_FAULT, OUTPUT, false);
    BSP_GPIO_Init(MOTOR_CONTACTOR_PORT, MOTOR_CONTACTOR, OUTPUT, false);
    BSP_GPIO_Init(MOTOR_PRCHG_BYPASS_PORT, MOTOR_PRCHG_BYPASS, OUTPUT, false);
    BSP_GPIO_Init(ARRAY_PRCHG_BYPASS_PORT, ARRAY_PRCHG_BYPASS, OUTPUT, false);
    BSP_GPIO_Init(MOTOR_CTRL_FAULT_PORT, MOTOR_CTRL_FAULT, OUTPUT, false);
    BSP_GPIO_Init(BPS_FAULT_PORT, BPS_FAULT, OUTPUT, false);
    BSP_GPIO_Init(CONTROLS_FAULT_PORT, CONTROLS_FAULT, OUTPUT, false);
    BSP_GPIO_Init(CRUISE_IND_PORT, CRUISE_IND, OUTPUT, false);
} 

/**
 * @brief   Writes data to a specified pin
 * @param   led The led to write to
 * @param   state true=ON or false=OFF
 * @return  None
 */ 
void Status_Leds_Write(status_led_t led, bool state){
    switch (led){
        case OS_FAULT_LED:
            BSP_GPIO_Write_Pin(OS_FAULT_PORT, OS_FAULT, state);
            break;
        case MOTOR_CONTACTOR_LED:
            BSP_GPIO_Write_Pin(MOTOR_CONTACTOR_PORT, MOTOR_CONTACTOR, state);
            break;
        case MOTOR_PRECHARGE_CONTACTOR_LED:
            BSP_GPIO_Write_Pin(MOTOR_PRCHG_BYPASS_PORT, MOTOR_PRCHG_BYPASS, state);
            break;
        case ARRAY_PRECHARGE_CONTACTOR_LED:
            BSP_GPIO_Write_Pin(ARRAY_PRCHG_BYPASS_PORT, ARRAY_PRCHG_BYPASS, state);
            break;
        case MOTOR_CONTROLLER_FAULT_LED:
            BSP_GPIO_Write_Pin(MOTOR_CTRL_FAULT_PORT, MOTOR_CTRL_FAULT, state);
            break;
        case BPS_FAULT_LED:
            BSP_GPIO_Write_Pin(BPS_FAULT_PORT, BPS_FAULT, state);
            break;
        case CONTROLS_FAULT_LED:
            BSP_GPIO_Write_Pin(CONTROLS_FAULT_PORT, CONTROLS_FAULT, state);
            break;
        case CRUISE_IND_LED:
            BSP_GPIO_Write_Pin(CRUISE_IND_PORT, CRUISE_IND, state);
            break;
        default:
            break;
    }
}

/**
 * @brief   Toggles a status led
 * @param   led The led to toggle
 * @return  None
 */ 
void Status_Leds_Toggle(status_led_t led){
    switch (led){
        case OS_FAULT_LED:
            BSP_GPIO_Toggle_Pin(OS_FAULT_PORT, OS_FAULT);
            break;
        case MOTOR_CONTACTOR_LED:
            BSP_GPIO_Toggle_Pin(MOTOR_CONTACTOR_PORT, MOTOR_CONTACTOR);
            break;
        case MOTOR_PRECHARGE_CONTACTOR_LED:
            BSP_GPIO_Toggle_Pin(MOTOR_PRCHG_BYPASS_PORT, MOTOR_PRCHG_BYPASS);
            break;
        case ARRAY_PRECHARGE_CONTACTOR_LED:
            BSP_GPIO_Toggle_Pin(ARRAY_PRCHG_BYPASS_PORT, ARRAY_PRCHG_BYPASS);
            break;
        case MOTOR_CONTROLLER_FAULT_LED:
            BSP_GPIO_Toggle_Pin(MOTOR_CTRL_FAULT_PORT, MOTOR_CTRL_FAULT);
            break;
        case BPS_FAULT_LED:
            BSP_GPIO_Toggle_Pin(BPS_FAULT_PORT, BPS_FAULT);
            break;
        case CONTROLS_FAULT_LED:
            BSP_GPIO_Toggle_Pin(CONTROLS_FAULT_PORT, CONTROLS_FAULT);
            break;
        case CRUISE_IND_LED:
            BSP_GPIO_Toggle_Pin(CRUISE_IND_PORT, CRUISE_IND);
            break;
        default:
            break;
    }
}

/**
 * @brief   Turns all status LEDs on
 * @return  None
 */ 
void Status_Leds_All_On(void){
    for(int i = 0; i < NUM_STATUS_LED; i++){
        Status_Leds_Write(i, ON);
    }
}