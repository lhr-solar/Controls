#ifndef SWITCHES_H
#define SWITCHES_H
#include "BSP_SPI.h"
#include "BSP_GPIO.h"
#include "common.h"
#include "config.h"
#include "GPIOExpander.h"

/**
 * Switches enum
 * 
 * All of our switches names
*/
typedef enum {
    CRUZ_SW=0,
    CRUZ_EN,
    HZD_SW, 
    FWD_SW, 
    HEADLIGHT_SW, 
    LEFT_SW, 
    RIGHT_SW, 
    REGEN_SW, 
    IGN_1, 
    IGN_2, 
    REV_SW
} switches_t;


/**
 * Switches that pertain to lights
*/
typedef struct {
    State LEFT_SW;
    State RIGHT_SW;
    State HEADLIGHT_SW;
    State HZD_SW;
}light_switches_t;

/**
 * Switches that pertain to velocity
*/
typedef struct {
    State CRUZ_SW;
    State CRUZ_EN;
    State FWD_SW;
    State REV_SW;
    State REGEN_SW;
}velocity_switches_t;



/**
 * Switch States
 * 
 * Stores the current state all of
 * the switches that control this system
 */

typedef struct {
    State IGN_1;
    State IGN_2;
    velocity_switches_t velDispSwitches;
    light_switches_t lightSwitches;
}switch_states_t;




/**
 * @brief   Initializes all switches
 *          from the steering wheel
 * @param   None
 * @return  None
 */ 
void Switches_Init(void);

/**
 * @brief   Reads the current state of 
 *          the specified switch
 * @param   sw the switch to read
 * @return  State of the switch (ON/OFF)
 */ 
State Switches_Read(switches_t sw);

#endif