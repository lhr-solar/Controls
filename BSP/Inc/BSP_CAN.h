/**
 * @file BSP_CAN.h
 * @brief This module provides low-level access to the Leaderboard's two CAN interfaces, 
 * intended to be used for car CAN and motor CAN.
 */

#ifndef BSP_CAN_H
#define BSP_CAN_H

#include "common.h"

#define BSP_CAN_DATA_LENGTH 8
#define BSP_CAN_NUM_MAILBOX 3

/**
 * @brief   The CAN bus to use.
 * @note    CAN_3 is the CAN bus that communicates with the motor controller.
 *         CAN_1 is the CAN bus that communicates with the car.
*/
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
 * @brief   Transmits the data onto the CAN bus with the specified id
 * @param   bus : The bus to transmit the data onto. Should only be either CAN_1 or CAN_3.
 * @param   id : Message of ID. Also indicates the priority of message. The lower the value, the higher the priority.
 * @param   data : data to be transmitted. The max is 8 bytes.
 * @param   len : num of bytes of data to be transmitted. This must be <= 8 bytes or else the rest of the message is dropped.
 * @return  ERROR if module was unable to transmit the data onto the CAN bus. SUCCESS indicates data was transmitted.
 */
ErrorStatus BspCanWrite(Can bus, uint32_t id,
                        const uint8_t data[BSP_CAN_DATA_LENGTH], uint8_t len);

/**
 * @brief   Gets the data that was received from the CAN bus.
 * @note    Non-blocking statement
 * @pre     The data parameter must be at least 8 bytes or hardfault may occur.
 * @param   id : pointer to store id of the message that was received.
 * @param   data : pointer to store data that was received. Must be 8bytes or bigger.
 * @return  ERROR if nothing was received so ignore id and data that was received. SUCCESS indicates data was received and stored.
 */
ErrorStatus BspCanRead(Can bus, uint32_t* id, uint8_t* data);

#endif