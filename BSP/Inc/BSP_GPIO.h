#ifndef __BSP_GPIO_H
#define __BSP_GPIO_H

#include "common.h"
#include "config.h"
#include <bsp.h>

typedef enum {PA0 = 0, PA2, PA3, PB0, PB1, PB2, PB4, PB5, PB6, PB7, PB8, PB14, PB15} Pin; 

#define GPIOSIZE 13

/**
 * @brief   Confirms that the CSV file
 *          has been created and throws
 *          an error if not
 * @param   None
 * @return  None
 */ 
void BSP_GPIO_Init(void);

/**
 * @brief   
 * 
 * @param   
 * 
 * @return  
 */ 
int BSP_GPIO_Read(Pin pn);

/**
 * @brief   
 * 
 * @param   
 * 
 * @return  
 */ 
int BSP_GPIO_Write(Pin pn, int data);

#endif