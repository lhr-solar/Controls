#ifndef __SENDCARCAN_H
#define __SENDCARCAN_H

#include "CANbus.h"
#include "common.h"
#include "os_cfg_app.h"
#include "CANbus.h"
#include "Minions.h"
#include "Contactors.h"
#include "Pedals.h"
#include "Ignition.h"
#include "Tasks.h"
#include "SendTritium.h"

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