#ifndef _IGNITION_H__
#define _IGNITION_H__

#include "daybreak_pins.h"
#include "BSP_GPIO.h"

/**
 * Possible Ignition Status
 */
typedef enum{
    IGN_OFF = 0,
    IGN_ARR,
    IGN_MOTOR,
    IGN_ERROR
}ignition_state_t;



#define ARRAY_IGNITION_PORT IG1_PORT
#define ARRAY_IGNITION_PIN  IG1

#define MOTOR_IGNITION_PORT IG2_PORT
#define MOTOR_IGNITION_PIN  IG2

/**
 * @brief Initializes the Ignition pins
 * 
 */
void Ignition_Init(void);

/**
 * @brief   Toggles a status led
 * @return  state of the ignition switch
 */ 
ignition_state_t Get_Ignition_State(void);

#endif