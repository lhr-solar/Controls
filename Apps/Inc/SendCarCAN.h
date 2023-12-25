/**
 * @file SendCarCAN.h
 * @brief The Send Car CAN task is a simple queue consumer task. Multiple 
 * tasks that need to write the the car CAN bus; in order to do this safely, 
 * they append their messages to a CAN queue. The Send Car CAN task simply pends 
 * on this queue and forwards messages to the Car CAN bus when any arrive.
 * 
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