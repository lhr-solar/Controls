/**
 * Header file for the library to interact
 * with the timers
 */
#include "common.h"
#include <bsp.h>

/** 
 * @brief   Updates the time by reading the CSV file and calling relevant functions at the right time 
 * @param   none
 * @return  none
 */  
void BSP_Timer_Update();

/** 
 * @brief   Intializes the Timer
 * @param   2 reload values for two timers, Two function pointers 
 * @return  none
 */  
void BSP_Timer_Init(int Timer1Reload,int Timer2Reload, void *func1, void *func2);

