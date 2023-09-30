/**
 * @file
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @brief Queue that holds all CAN messages that Task_CANBusConsumer needs to
 * send.
 *
 * @defgroup CAN_Queue
 * @addtogroup CAN_Queue
 * @{
 */

#ifndef CAN_QUEUE_H
#define CAN_QUEUE_H

#include "CANbus.h"

void CAN_Queue_Init(void);

ErrorStatus CAN_Queue_Post(CANDATA_t message);

ErrorStatus CAN_Queue_Pend(CANDATA_t *message);

#endif

/* @} */
