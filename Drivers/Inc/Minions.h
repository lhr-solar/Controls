/* Copyright (c) 2020 UT Longhorn Racing Solar */

#ifndef MINIONS_H
#define MINIONS_H
#include "BSP_SPI.h"
#include "BSP_GPIO.h"
#include "common.h"
#include "config.h"
#include "GPIOExpander.h"
#include "Tasks.h"

#define SPI_CS          GPIO_Pin_4
#define LIGHTS_PORT     PORTC
#define HEADLIGHT_PIN   GPIO_Pin_6
#define BRAKELIGHT_PIN  GPIO_Pin_7
#define RIGHT_BLINK_PIN GPIO_Pin_8
#define LEFT_BLINK_PIN  GPIO_Pin_9

typedef enum {
     HZD_SW=0, 
     CRUZ_EN, 
     REV_SW,
     FOR_SW,
     HEADLIGHT_SW,
     LEFT_SW,
     RIGHT_SW,
     REGEN_SW,
     CRUZ_ST,
     IGN_1,
     IGN_2
} switches_t;

// Ordered by pin of minion board
//TODO: Double check that this ordering remains valid
typedef enum {  
    A_CNCTR = 0,
    M_CNCTR,
    CTRL_FAULT,
    LEFT_BLINK,
    RIGHT_BLINK,
    Headlight_ON,
    BrakeLight,
    RSVD_LED
} light_t;

/**
 * @brief   Initializes all Lights and Switches
 * @param   None
 * @return  None
 */ 
void Minions_Init(void);

/**
 * @brief   Reads the current state of 
 *          the specified switch
 * @param   sw the switch to read
 * @return  bool of the switch (true/false)
 */ 
bool Switches_Read(switches_t sw);

/**
* @brief   Read the state of the lights
* @param   light Which Light to read
* @return  returns state enum which indicates true/false
*/ 
bool Lights_Read(light_t light);

/**
 * @brief   Sends SPI messages to read switches values. Also reads from GPIO's for 
 *          ignition switch values
 */ 
void Switches_UpdateStates(void);

/**
 * @brief   Set light to given state
 * @param   light which light to set
 * @param   state what state to set the light to
 * @return  void
 */
void Lights_Set(light_t light, bool state);

/**
* @brief   Read the lights bitmap
* @return  returns uint16_t with lights bitmap
*/ 
uint16_t Lights_Bitmap_Read();

/**
 * @brief Toggles a light. Should be used only after Toggle_Enable has been called for this light so that we are accurately tracking the enabled and disabled lights
 * @param light Which light to toggle
*/
void Lights_Toggle(light_t light);

/**

 * @brief   Set light toggling
 * @param   light Which light to enable toggling for
 * @param   state bool to set toggling
 * @return  void
 */
void Lights_Toggle_Set(light_t light, bool state);

/**
* @brief   Read the toggle enable state of a specific light from the toggle bitmap
* @param   light Which Light to read
* @return  returns bool enum which indicates true/false
*/
bool Lights_Toggle_Read(light_t light);

/**
 * @brief   Read toggle bitmap
 * @return  returns uint8_t bitmap for toggle (LEFT/RIGHT)
 */
uint8_t Lights_Toggle_Bitmap_Read(void);

#endif
