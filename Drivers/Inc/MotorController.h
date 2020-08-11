#ifndef __MOTORCONTROLLER_H
#define __MOTORCONTROLLER_H

#include <bsp.h>

/**
 * @brief   Initializes the motor controller
 * @param   None
 * @return  None
 */ 
void Motor_Controller_Init(void);

/**
 * @brief   Sends MOTOR DRIVE command on CAN2
 * @param   newVelocity desired motor velocity setpoint in m/s
 * @param   motorCurrent desired motor current setpoint as a percentage of max current setting
 * @return  None
 */ 
void Motor_Controller_Drive(uint32_t newVelocity, uint32_t motorCurrent);

/**
 * @brief   Reads VELOCITY command from CAN2
 * @param   rpmBuff the car's velocity in rpm
 * @param   mpsBuff the car's velocity in m/s
 * @return  true if the ID on the bus is the expected ID for VELOCITY
 */ 
bool Motor_Controller_ReadVelocity(uint32_t *rpmBuff, uint32_t *mpsBuff);

#endif