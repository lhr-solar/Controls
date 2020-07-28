/**
 * Header file for the library to interact
 * with both CAN lines in the car
 */

#ifndef __BSP_CAN_H
#define __BSP_CAN_H

#include "common.h"
#include <bsp.h>

typedef enum {CAN1=0, CAN2} CAN_t;

/**
 * @brief   Initializes both CAN lines to
 *          communicate with the motor controllers
 *          and other car systems
 * @param   None
 * @return  None
 */ 
void BSP_CAN_Init(void);

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
 * @param   id pointer to integer to store the 
 *          message ID that was read
 * @param   data pointer to integer array to store
 *          the message in bytes
 * @return  number of bytes read (0 if unsuccessful)
 */
uint8_t BSP_CAN_Read(CAN_t bus, uint32_t* id, uint8_t* data);

#endif
