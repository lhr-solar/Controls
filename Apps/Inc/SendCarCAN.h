/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file SendCarCAN.h
 * @brief Function prototypes for the SendCarCAN application.
 * 
 * SendCarCAN contains functions relevant to placing CAN messages in a CarCAN queue and 
 * periodically sending those messages in the SendCarCAN task.
*/
#ifndef __SENDCARCAN_H
#define __SENDCARCAN_H

#include "CANbus.h"

/**
 * @brief Initialize SendCarCAN
*/
void SendCarCAN_Init();

/**
 * @brief Wrapper to put new message in the CAN queue
 * @param message the CAN message to put in SendCarCAN Queue
*/
void SendCarCAN_Put(CANDATA_t message);

#ifdef DEBUG
/**
 * @brief Return the space left in SendCarCAN_Q for debug purposes
 * @return the space left in SendCarCAN_Q
*/
uint8_t get_SendCarCAN_Q_Space(void);
#endif

#endif