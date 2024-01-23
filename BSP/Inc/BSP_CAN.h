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

#ifndef BSP_CAN_H
#define BSP_CAN_H

#include "common.h"

#define BSP_CAN_DATA_LENGTH 8
#define BSP_CAN_NUM_MAILBOX 3

typedef enum { kCan1 = 0, kCan3, kNumCan } Can;

/**
 * @brief   Initializes the CAN module that communicates with the rest of the
 * electrical system.
 * @param   bus : The bus to initialize. Should only be either CAN_1 or CAN_3.
 * @param   rxEvent : the function to execute when recieving a message. NULL for
 * no action.
 * @param   txEnd : the function to execute after transmitting a message. NULL
 * for no action.
 * @param   idWhitelist : the idWhitelist to use for message filtering. NULL for
 * no filtering.
 * @param   idWhitelistSize : the size of the idWhitelist, if it is not NULL.
 * @return  None
 */
void BspCanInit(Can bus, Callback rx_event, Callback tx_end,
                uint16_t* id_whitelist, uint8_t id_whitelist_size);

/**
 * @brief   Writes a message to the specified CAN line
 * @param   bus the proper CAN line to write to
 *          defined by the CAN_t enum
 * @param   id the hex ID for the message to be sent
 * @param   data pointer to the array containing the message
 * @param   len length of the message in bytes
 * @return  number of bytes transmitted (0 if unsuccessful)
 */

ErrorStatus BspCanWrite(Can bus, uint32_t id,
                        const uint8_t data[BSP_CAN_DATA_LENGTH], uint8_t len);

/**
 * @brief   Reads the message on the specified CAN line
 * @param   id pointer to integer to store the
 *          message ID that was read
 * @param   data pointer to integer array to store
 *          the message in bytes
 * @return  number of bytes read (0 if unsuccessful)
 */
ErrorStatus BspCanRead(Can bus, uint32_t* id, uint8_t* data);

#endif

/* @} */
