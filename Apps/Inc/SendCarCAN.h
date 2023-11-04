#ifndef __SENDCARCAN_H
#define __SENDCARCAN_H

#include "CANbus.h"

/**
 * @brief Initialize SendCarCAN
*/
void SendCarCAN_Init();

/**
 * @brief Wrapper to put new message in the CAN queue
*/
void SendCarCAN_Put(CANDATA_t message);

/**
 * @brief print SendCarCAN_Q put/get for debug purposes
*/
void print_SendCarCAN_Q(void);

#endif