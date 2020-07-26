/**
 * Header file for the library to interact
 * with the switches on the steering wheel
 */

#ifndef __BSP_CAN_H
#define __BSP_CAN_H

#include "common.h"
#include <bsp.h>


/**
 * @brief   Initializes both CAN lines to
 *          communicate with the motor controllers
 *          and other car systems
 * @param   None
 * @return  None
 */ 
void BSP_CAN_Init(void);

uint8_t BSP_CAN1_Write(uint32_t id, uint8_t data[8], uint8_t len);

uint8_t BSP_CAN1_Read(uint32_t *id, uint8_t *data);

uint8_t BSP_CAN2_Write(uint32_t id, uint8_t data[8], uint8_t len);

uint8_t BSP_CAN2_Read(uint32_t *id, uint8_t *data);

#endif
