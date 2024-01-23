/**
 * @file SendCarCAN.h
 * @brief Sends any CarCAN messages put in the queue via SendCarCAN_Put, and
 * handles transmitting the IO State of the car for telemetry & ignition
 * sequence.
 *
 * Call SendCarCAN_Init() to initialize the queue and semaphore. Call
 * SendCarCAN_Put() to put a message in the queue. Starting the SendCarCAN task
 * will send any messages put in the queue, as well as spawn PutIOState task to
 * forward the IO state.
 *
 */

#ifndef SENDCARCAN_H
#define SENDCARCAN_H

#include "CanBus.h"

/**
 * @brief Initialize SendCarCAN
 */
void SendCarCanInit();

/**
 * @brief Wrapper to put new message in the CAN queue
 * @param message the CAN message to put in SendCarCAN Queue
 */
void SendCarCanPut(CanData message);

#ifdef DEBUG
/**
 * @brief Return the space left in SendCarCAN_Q for debug purposes
 * @return the space left in SendCarCAN_Q
 */
uint8_t GetSendCarCanQueueSpace(void);
#endif

#endif