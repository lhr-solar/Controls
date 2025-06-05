#ifndef __SENDCARCAN_H
#define __SENDCARCAN_H

#include "common.h"
#include "os.h"
#include "CANbus.h"
#include "Tasks.h"



/**
 * @brief Initialize SendCarCAN
*/
void SendCarCAN_Init();

/**
 * @brief Wrapper to put new message in the CAN queue
*/
void SendCarCAN_Put(CANDATA_t message);

/**
 * @brief return the space left in SendCarCAN_Q for debug purposes
*/
#ifdef DEBUG
uint8_t get_SendCarCAN_Q_Space(void);
#endif

#endif