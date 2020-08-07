#ifndef __MOTORCONTROLLER_H
#define __MOTORCONTROLLER_H

#include <bsp.h>

/**
 * @brief   Initializes the motor controller
 * @param   None
 * @return  None
 */ 
void Motor_Controller_Init();

/**
 * @brief   Sends MOTOR DRIVE command on CAN2
 * @param   rpmBuff the car's velocity in rpm
 * @param   mpsBuff the car's velocity in m/s
 * @return  None
 */ 
void Motor_Controller_Drive(uint32_t *rpmBuff, uint32_t *mpsBuff);

/**
 * @brief   Reads VELOCITY command from CAN2
 * @param   None
 * @return  true if the ID on the bus is the expected ID for VELOCITY
 */ 
bool Motor_Controller_ReadVelocity();

#endif