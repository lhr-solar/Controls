/* Copyright (c) 2021 UT Longhorn Racing Solar */
/** @defgroup CAN_Queue 
 * Queue that holds all CAN messages that Task_CANBusConsumer needs to send.
*/
/** @ingroup CAN_Queue 
 * @{ */

#ifndef CAN_QUEUE_H
#define CAN_QUEUE_H

#include "CANbus.h"

void CAN_Queue_Init(void);

ErrorStatus CAN_Queue_Post(CANDATA_t message);

ErrorStatus CAN_Queue_Pend(CANDATA_t *message);

#endif

/*@}*/
