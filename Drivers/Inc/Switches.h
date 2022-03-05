#ifndef SWITCHES_H
#define SWITCHES_H
#include "BSP_SPI.h"
#include "BSP_GPIO.h"
#include "common.h"
#include "config.h"
#include "GPIOExpander.h"

#define SPI_CS GPIO_Pin_4

typedef enum {CRUZ_ST=0, CRUZ_EN, REV_SW, FOR_SW, HEADLIGHT_SW, LEFT_SW, RIGHT_SW, REGEN_SW, HZD_SW, IGN_1, IGN_2} switches_t;

/**
 * Switch States
 * 
 * Stores the current state of each of
 * the switches that control this system
 */
typedef struct {
    State LT;
    State RT;
    State FWD;
    State REV;
    State CRS_EN;
    State CRS_SET;
    State REGEN;
    State HZD;
    State HDLT;
    State IGN_1;
    State IGN_2;
} switch_states_t;

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

/**
 * @brief   Sends SPI messages to read switches values. Also reads from GPIO's for 
 *          ignition switch values
 */ 
void Switches_UpdateStates(void);

#endif