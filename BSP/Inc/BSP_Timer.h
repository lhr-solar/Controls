/**
 * Header file for the library to interact
 * with the timers
 */
#include "common.h"
#include <bsp.h>

typedef enum {TIME_1=0, TIME_2} Time_t;


/** 
 * @brief   Updates the time by reading the CSV file and calling relevant functions at the right time 
 * @param   none
 * @return  none
 */  
void BSP_Timer_Update();

/** 
 * @brief   Intializes the Timer
 * @param   Reload vlaue of Timer
 * @param   Pointer to function
 * @param   Specefic Timer
 * @return  none
 */  
void BSP_Timer_Init(int TimerReload,void *func1, Time_t time);

