/* Copyright (c) 2021 UT Longhorn Racing Solar */
/** CAN_Queue.c
 * Queue that holds all CAN messages that Task_CANBusConsumer needs to send.
 */

#ifndef CAN_QUEUE_H
#define CAN_QUEUE_H

#include "CANbus.h"

void CAN_Queue_Init(void);

ErrorStatus CAN_Queue_Post(CANMSG_t message);

ErrorStatus CAN_Queue_Pend(CANMSG_t *message);

#endif
