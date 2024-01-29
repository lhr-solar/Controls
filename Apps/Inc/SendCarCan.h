/**
 * @file SendCarCan.h
 * @brief Sends any CarCAN messages put in the queue via SendCarCanPut, and
 * handles transmitting the IO State of the car for telemetry & ignition
 * sequence.
 *
 * Call SendCarCanInit() to initialize the queue and semaphore. Call
 * SendCarCanPut() to put a message in the queue. Starting the SendCarCan task
 * will send any messages put in the queue, as well as spawn PutIOState task to
 * forward the state of all inputs/outputs under the Controls system for
 * telemetry.
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
 * @brief Return the space left in the queue for debug purposes
 * @return the space left in the queue
 */
uint8_t GetSendCarCanQueueSpace(void);
#endif

#endif