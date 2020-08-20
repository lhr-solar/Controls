#ifndef __MOTORCONTROLLER_H
#define __MOTORCONTROLLER_H

#include <bsp.h>
#include "CAN.h"

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
 * @brief   Reads desired command from CAN2
 * @param   message the car's information regarding the desired command
 * @param   expectedID the id for the command the user wants to check CAN2 for
 * @return  true if the ID on the bus is the expected ID 
 */ 
bool MotorController_Read(CANbuff *message, uint32_t expectedID);

#endif