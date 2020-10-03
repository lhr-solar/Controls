/* Copyright (c) 2020 UT Longhorn Racing Solar */

/**
 * Header file for the library to interact
 * with the timers
 */

#ifndef __BSP_TIMER_H
#define __BSP_TIMER_H

#include "common.h"
#include <bsp.h>

typedef enum {TIMER_1=0, TIMER_2, NUM_TIMERS} ticker_t;

/** 
 * @brief   Intializes the Timer
 * @param   reload value of Timer
 * @param   handler interrupt handler
 * @param   timer specified by ticker_t
 * @return  None
 */  
void BSP_Timer_Init(int reload, void (*handler) (void), ticker_t timer);

/** 
 * @brief   Updates the time by reading the CSV file
 *          and calling relevant functions at the right time.
 *          This function is only necessary for the simulator
 * @param   None
 * @return  None
 */  
void BSP_Timer_Update();

#endif
