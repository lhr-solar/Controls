#ifndef SWITCHES_H
#define SWITCHES_H
#include "BSP_SPI.h"
#include "common.h"
#include "config.h"


typedef enum {CRUZ_st=0, CRUZ_EN, HZD_SQ, FR_SW, Headlight_SW, LEFT_SW, RIGHT_SW, RSVD_LED} switches_t;

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