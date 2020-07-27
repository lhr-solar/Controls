/**
 * Header file for the library to interact
 * with both CAN lines in the car
 */

#ifndef __BSP_CAN_H
#define __BSP_CAN_H

#include "common.h"
#include <bsp.h>
#include <math.h>

typedef enum {CAN1=0, CAN2} CAN_t;

/**
 * @brief   Initializes both CAN lines to
 *          communicate with the motor controllers
 *          and other car systems
 * @param   None
 * @return  None
 */ 
void BSP_CAN_Init(void);

uint8_t BSP_CAN_Write(CAN_t bus, uint32_t id, uint8_t* data, uint8_t len);

uint8_t BSP_CAN_Read(CAN_t bus, uint32_t* id, uint8_t* data);

#endif
