/* Copyright (c) 2020 UT Longhorn Racing Solar */

#ifndef __MOTORCONTROLLER_H
#define __MOTORCONTROLLER_H

#include <bsp.h>

typedef struct 
{
    uint32_t id;
    union {
    uint32_t firstNum;
    uint8_t data1[4];
    };
    union {
    uint32_t secondNum;
    uint8_t data2[4];
    };
} CANbuff;

/**
 * Motor Error States
 * Read messages from motor in ReadTritium and trigger appropriate error messages as needed based on bits
 * 
 */
typedef enum{
    T_NONE = 0x00,
    T_TEMP_ERR = 0x01,
    T_CC_VEL_ERR = 0x02,
    T_SLIP_SPEED_ERR = 0x04,
    T_OVER_SPEED_ERR = 0x08,
    T_INIT_FAIL = 0x16
} tritium_error_code_t;

/**
 * @brief   Initializes the motor controller
 * @param   busCurrentFractionalSetPoint fraction of the bus current to allow the motor to draw
 * @return  None
 */ 
void MotorController_Init(float busCurrentFractionalSetPoint);

/**
 * @brief   Sends MOTOR DRIVE command on CAN3
 * @param   newVelocity desired motor velocity setpoint in rpm
 * @param   motorCurrent desired motor current setpoint as a fraction of max current setting
 * @return  None
 */ 
void MotorController_Drive(float newVelocity, float motorCurrent);

/**
 * @brief   Reads most recent command from CAN3 bus
 * @param   message the buffer in which the info for the CAN message will be stored
 * @return  SUCCESS if a message is read
 */ 
ErrorStatus MotorController_Read(CANbuff *message);

/**
 * @brief   Reads velocity
 * @return  Velocity
 */ 
float MotorController_ReadVelocity(void);


#endif
