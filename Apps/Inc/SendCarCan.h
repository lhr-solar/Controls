#ifndef SENDCARCAN_H
#define SENDCARCAN_H

#include "CanBus.h"

/**
 * Task Prototype
 */
void TaskSendCarCan(void* p_arg);

/**
 * @brief Initialize SendCarCAN
 */
void SendCarCanInit();

/**
 * @brief Wrapper to put new message in the CAN queue
 */
void SendCarCanPut(CanData message);

/**
 * @brief return the space left in SendCarCAN_Q for debug purposes
 */
#ifdef DEBUG
uint8_t GetSendCarCanQueueSpace(void);
#endif

#endif