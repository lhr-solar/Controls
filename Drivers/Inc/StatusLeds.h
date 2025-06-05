/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file StatusLeds.h
 * @brief Header file for the StatusLeds driver
 */

#ifndef __STATUS_LEDS_H
#define __STATUS_LEDS_H

#include "common.h"


/**
 * Possible Status LED Pins
 */
typedef enum {
    ARRAY_PRECHARGE_CONTACTOR_LED = 0,
    MOTOR_PRECHARGE_CONTACTOR_LED,
    OS_FAULT_LED,
    CONTROLS_FAULT_LED,
    BPS_FAULT_LED,
    MOTOR_CONTROLLER_FAULT_LED,
    CRUISE_IND_LED,
    DASH_BPS_HAZ_LED,
    DASH_HEARTBEAT_LED,
    BRAKELIGHT_LED,
    NUM_STATUS_LED,
} status_led_t;
 
/**
 * @brief Initializes Status LED pins
 * 
 */
void Status_Leds_Init(void);

/**
 * @brief   Writes data to a specified pin
 * @param   led The led to write to
 * @param   state true=ON or false=OFF
 * @return  None
 */ 
void Status_Leds_Write(status_led_t led, bool state);

/**
 * @brief   Toggles a status led
 * @param   led The led to toggle
 * @return  None
 */ 
void Status_Leds_Toggle(status_led_t led);

/**
 * @brief   Turns all status LEDs on
 * @return  None
 */ 
void Status_Leds_All_On(void);

/**
 * @brief   Turns all status LEDs off
 * @return  None
 */ 
void Status_Leds_All_Off(void);


#endif
