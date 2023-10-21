/**
 * @copyright Copyright (c) 2023 UT Longhorn Racing Solar
 * 
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
#include "Minions.h"
#include "Contactors.h"
#include "Pedals.h"
#include "Tasks.h"
#include "SendCarCAN.h"
#include "SendTritium.h"

#define IO_STATE_DLY_MS 250u 
#define SENDCARCAN_LOOP_DLY_MS 50u // How often we should check the CAN queue (in ms delay). Must send faster than queue messages get put in
#define IO_STATE_DLY_COUNT (IO_STATE_DLY_MS / SENDCARCAN_LOOP_DLY_MS) // The number of periods to wait before sending IO state message
#define IO_STATE_TMR_DLY_TS ((IO_STATE_TMR_DLY_MS * OS_CFG_TMR_TASK_RATE_HZ) / (1000u))// **** delete?

// fifo
//#define FIFO_TYPE CANDATA_t
//#define FIFO_SIZE 256
//#define FIFO_NAME SendCarCAN_Q
//#include "fifo.h"

#ifdef __TEST_SENDCARCAN
#define SCOPE
#else
#define SCOPE static
#endif 

SCOPE SendCarCAN_Q_t CANFifo; 

static OS_SEM CarCAN_Sem4;
static OS_MUTEX CarCAN_Mtx;

static uint8_t IOStateDlyCounter = 0;


static void putIOState(void);

/**
 * @brief Wrapper to put new message in the CAN queue
*/
void SendCarCAN_Put(CANDATA_t message){
    OS_ERR err;
    CPU_TS ticks;
    
    OSMutexPend(&CarCAN_Mtx, 0, OS_OPT_PEND_BLOCKING, &ticks, &err);
    assertOSError(OS_SEND_CAN_LOC, err);
    
    bool success = SendCarCAN_Q_put(&CANFifo, message);

    OSMutexPost(&CarCAN_Mtx, OS_OPT_POST_NONE, &err);
    assertOSError(OS_SEND_CAN_LOC, err);

    if(success) OSSemPost(&CarCAN_Sem4, OS_OPT_POST_1, &err);
    assertOSError(OS_SEND_CAN_LOC, err);
}

/**
 * @brief Grabs the latest messages from the queue and sends over CarCAN
*/
void Task_SendCarCAN(void *p_arg){
    OS_ERR err;
    CPU_TS ticks;
    
    OSMutexCreate(&CarCAN_Mtx, "CarCAN_Mtx", &err);
    assertOSError(OS_SEND_CAN_LOC, err);
    
    OSSemCreate(&CarCAN_Sem4, "CarCAN_Sem4", 0, &err);
    assertOSError(OS_SEND_CAN_LOC, err);

    SendCarCAN_Q_renew(&CANFifo);
    
    // Set up timer to put car data message every IO_STATE_TMR_DLY_MS ms //***************


    CANDATA_t message;
    memset(&message, 0, sizeof(CANDATA_t));

    while (1) {
        
  
        if (++IOStateDlyCounter >= IO_STATE_DLY_COUNT) { // Send IO State message approximately every IO_STATE_DLY_MS
            IOStateDlyCounter = 0;
            putIOState();
        }
        
        
        // Check if there's something to send in the queue (either IOState or Car state from sendTritium)
        OSSemPend(&CarCAN_Sem4, 0, OS_OPT_PEND_NON_BLOCKING, &ticks, &err);
        if (err != OS_ERR_PEND_WOULD_BLOCK) { // We have a message in the queue to send
            assertOSError(OS_SEND_CAN_LOC, err);

            OSMutexPend(&CarCAN_Mtx, 0, OS_OPT_PEND_BLOCKING, &ticks, &err);
            assertOSError(OS_SEND_CAN_LOC, err);
        
            bool res = SendCarCAN_Q_get(&CANFifo, &message);
            assertOSError(OS_SEND_CAN_LOC, err);

            OSMutexPost(&CarCAN_Mtx, OS_OPT_POST_NONE, &err);

            if(res) CANbus_Send(message, true, CARCAN);
        }
            
        OSTimeDlyHMSM(0, 0, 0, SENDCARCAN_LOOP_DLY_MS, OS_OPT_TIME_HMSM_STRICT, &err);
        assertOSError(OS_SEND_CAN_LOC, err);
        
    }
}

static void putIOState(void){
    CANDATA_t message;
    memset(&message, 0, sizeof(message));
    message.ID = IO_STATE;
    
    // Get pedal information
    message.data[0] = Pedals_Read(ACCELERATOR);
    message.data[1] = Pedals_Read(BRAKE);

    // Get minion information
    message.data[2] = 0;
    for(pin_t pin = 0; pin < NUM_PINS; pin++){
        bool pinState = Minions_Read(pin);
        message.data[2] |= pinState << pin;
    }
    
    // Get contactor info
    message.data[3] = 0;
    for(contactor_t contactor = 0; contactor < NUM_CONTACTORS; contactor++){
        bool contactorState = (Contactors_Get(contactor) == ON) ? true : false;
        message.data[3] |= contactorState << contactor;
    }

    SendCarCAN_Put(message);
}
