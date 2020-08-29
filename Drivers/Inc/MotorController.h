#ifndef __MOTORCONTROLLER_H
#define __MOTORCONTROLLER_H

#include <bsp.h>

typedef struct 
{
    uint32_t id;
    uint32_t firstNum;
    uint32_t secondNum;
} CANbuff;

/**
 * @brief   Initializes the motor controller
 * @param   None
 * @return  None
 */ 
void MotorController_Init(void);

/**
 * @brief   Sends MOTOR DRIVE command on CAN2
 * @param   newVelocity desired motor velocity setpoint in m/s
 * @param   motorCurrent desired motor current setpoint as a percentage of max current setting
 * @return  None
 */ 
void MotorController_Drive(uint32_t newVelocity, uint32_t motorCurrent);

/**
 * @brief   Reads most recent command from CAN2 bus
 * @param   message the buffer in which the info for the CAN message will be stored
 * @return  true if a message is read
 */ 
error_t MotorController_Read(CANbuff *message);

#endif