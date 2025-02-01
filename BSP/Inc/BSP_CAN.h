/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file BSP_CAN.h
 * @brief Header file for the library to interact
 * with both CAN lines in the car
 * 
 * @defgroup BSP_CAN
 * @addtogroup BSP_CAN
 * @{
 */

#ifndef __BSP_CAN_H
#define __BSP_CAN_H

#include "common.h"
#include "config.h"
#include <bsp.h>

typedef enum {CAN_2=0, CAN_3, NUM_CAN} CAN_t;

/**
 * @brief   Initializes the CAN module that communicates with the rest of the electrical system.
 * @param   bus : The bus to initialize. Should only be either CAN_1 or CAN_3.
 * @param   rxEvent : the function to execute when recieving a message. NULL for no action.
 * @param   txEnd : the function to execute after transmitting a message. NULL for no action.
 * @param   idWhitelist : the idWhitelist to use for message filtering. NULL for no filtering.
 * @param   idWhitelistSize : the size of the idWhitelist, if it is not NULL.
 * @return  None
 */
void BSP_CAN_Init(CAN_t bus, callback_t rxEvent, callback_t txEnd, uint16_t* idWhitelist, uint8_t idWhitelistSize);

/**
 * @brief   Writes a message to the specified CAN line
 * @param   bus the proper CAN line to write to
 *          defined by the CAN_t enum
 * @param   id the hex ID for the message to be sent
 * @param   data pointer to the array containing the message
 * @param   len length of the message in bytes
 * @return  number of bytes transmitted (0 if unsuccessful)
 */
ErrorStatus BSP_CAN_Write(CAN_t bus, uint32_t id, uint8_t data[8], uint8_t len);

/**
 * @brief   Reads the message on the specified CAN line
 * @param   id pointer to integer to store the 
 *          message ID that was read
 * @param   data pointer to integer array to store
 *          the message in bytes
 * @return  number of bytes read (0 if unsuccessful)
 */
ErrorStatus BSP_CAN_Read(CAN_t bus, uint32_t* id, uint8_t* data);

#endif


/* @} */
