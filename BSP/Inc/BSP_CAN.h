/* Copyright (c) 2020 UT Longhorn Racing Solar */

/**
 * Header file for the library to interact
 * with both CAN lines in the car
 */

#ifndef __BSP_CAN_H
#define __BSP_CAN_H

#include "common.h"
#include <bsp.h>

typedef enum {CAN_1=0, CAN_2, CAN_3, NUM_CAN} CAN_t;

/**
 * @brief   Initializes both CAN lines to
 *          communicate with the motor controllers
 *          and other car systems
 * @param   bus the CAN line to initialize
 * @param txHandler the Function that will be called every time TX completes
 * @param rxHanlder the function that will be called on every RX event
 * @return  None
 */ 
void BSP_CAN_Init(CAN_t bus,void (*txHandler)(void), void (*rxHander)(void));

/**
 * @brief   Writes a message to the specified CAN line
 * @param   bus the proper CAN line to write to
 *          defined by the CAN_t enum
 * @param   id the hex ID for the message to be sent
 * @param   data pointer to the array containing the message
 * @param   len length of the message in bytes
 * @return  number of bytes transmitted (0 if unsuccessful)
 */
uint8_t BSP_CAN_Write(CAN_t bus, uint32_t id, uint8_t* data, uint8_t len);

/**
 * @brief   Reads the message on the specified CAN line
 * @param   bus the bus line 
 * @param   id pointer to integer to store the 
 *          message ID that was read
 * @param   data pointer to integer array to store
 *          the message in bytes
 * @return  number of bytes read (0 if unsuccessful)
 */
uint8_t BSP_CAN_Read(CAN_t bus, uint32_t* id, uint8_t* data);

#endif
