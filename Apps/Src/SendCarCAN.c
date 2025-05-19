/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file SendCarCAN.c
 * @brief Function implementations for the SendCarCAN application.
 * 
 * This contains functions relevant to placing CAN messages in a CarCAN queue and periodically sending
 * those messages in the SendCarCAN task.
 * 
 */

#include "common.h"
#include "os_cfg_app.h"
#include "CANbus.h"
#include "Tasks.h"
#include "DebugIO.h"
#include "StatusLeds.h"
#include "SendCarCAN.h"
#include "SendTritium.h"

#define SENDCARCAN_MSG_SKIP_CTR 3

//fifo
#define FIFO_TYPE CANDATA_t
#define FIFO_SIZE 50
#define FIFO_NAME SendCarCAN_Q
#include "fifo.h"

static SendCarCAN_Q_t CANFifo; 

static OS_SEM CarCAN_Sem4;
static OS_MUTEX CarCAN_Mtx;


/**
 * @brief return the space left in SendCarCAN_Q for debug purposes
*/
#ifdef DEBUG
uint8_t get_SendCarCAN_Q_Space(void) {
    return (CANFifo.get - CANFifo.put - 1) % (sizeof CANFifo.buffer / sizeof CANFifo.buffer[0]);
}
#endif

/**
 * @brief Wrapper to put new message in the CAN queue
*/
void SendCarCAN_Put(CANDATA_t message){
    OS_ERR err;
    CPU_TS ticks;
    bool success = false;
    
    static uint8_t carcan_ctr = 0;
    
    if(carcan_ctr > SENDCARCAN_MSG_SKIP_CTR){
        OSMutexPend(&CarCAN_Mtx, 0, OS_OPT_PEND_BLOCKING, &ticks, &err);
        assertOSError(err);

        success = SendCarCAN_Q_put(&CANFifo, message);

        OSMutexPost(&CarCAN_Mtx, OS_OPT_POST_NONE, &err);
        assertOSError(err);

        carcan_ctr = 0;
    }
    carcan_ctr++;


    if(success) {
        OSSemPost(&CarCAN_Sem4, OS_OPT_POST_1, &err);
        assertOSError(err);
    }
}

/**
 * @brief Initialize SendCarCAN
*/
void SendCarCAN_Init(void) {
    OS_ERR err;
    
    OSMutexCreate(&CarCAN_Mtx, "CarCAN_Mtx", &err);
    assertOSError(err);
    
    OSSemCreate(&CarCAN_Sem4, "CarCAN_Sem4", 0, &err);
    assertOSError(err);

    SendCarCAN_Q_renew(&CANFifo);
}

/**
 * @brief Grabs the latest messages from the queue and sends over CarCAN
*/
void Task_SendCarCAN(void *p_arg){
    OS_ERR err;
    CPU_TS ticks;

    CANDATA_t message;
    memset(&message, 0, sizeof message);

    while (1) {
          
        // Check if there's something to send in the queue (either IOState or Car state from sendTritium)
        OSSemPend(&CarCAN_Sem4, 0, OS_OPT_PEND_BLOCKING, &ticks, &err);
        assertOSError(err);
        #ifdef TASK_PROFILER
        DebugIO_Toggle(SEND_CARCAN_PIN);
        #endif

        OSMutexPend(&CarCAN_Mtx, 0, OS_OPT_PEND_BLOCKING, &ticks, &err);
        assertOSError(err);
    
        bool res = SendCarCAN_Q_get(&CANFifo, &message);

        OSMutexPost(&CarCAN_Mtx, OS_OPT_POST_NONE, &err);
        assertOSError(err);

        #ifdef TASK_PROFILER
        DebugIO_Toggle(SEND_CARCAN_PIN);
        #endif

        if(res) CANbus_Send(message, true, CARCAN);
    }
}