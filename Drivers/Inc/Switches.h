#ifndef SWITCHES_H
#define SWITCHES_H
#include "BSP_SPI.h"
#include "BSP_GPIO.h"
#include "common.h"
#include "config.h"


typedef enum {CRUZ_SW=0, CRUZ_EN, HZD_SQ, FR_SW, HEADLIGHT_SW, LEFT_SW, RIGHT_SW, REGEN_SW, IGN_1, IGN_2, REV_SW} switches_t;

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