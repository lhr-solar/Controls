#ifndef __SENDCARCAN_H
#define __SENDCARCAN_H

#include "CANbus.h"

/**
 * @brief Initialize SendCarCAN
*/
void SendCarCAN_Init();
void SendCarCAN_Put(CANDATA_t message);
#define __TEST_SENDCARCAN
#ifdef __TEST_SENDCARCAN
#define FIFO_TYPE CANDATA_t
#define FIFO_SIZE 256
#define FIFO_NAME SendCarCAN_Q
#include "fifo.h"
extern SendCarCAN_Q_t CANFifo;
#endif
#endif