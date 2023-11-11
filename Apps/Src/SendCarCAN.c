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
#include "Minions.h"
#include "Contactors.h"
#include "Pedals.h"
#include "Tasks.h"
#include "SendCarCAN.h"
#include "SendTritium.h"

#define IO_STATE_DLY_MS 250u 

// Task_PutIOState
OS_TCB putIOState_TCB;
CPU_STK putIOState_Stk[TASK_SEND_CAR_CAN_STACK_SIZE];

//fifo
#define FIFO_TYPE CANDATA_t
#define FIFO_SIZE 256
#define FIFO_NAME SendCarCAN_Q
#include "fifo.h"

// Motor message
#define NUM_MOTOR_MSGS 15 // Messages received from the motor controller
#define MOTOR_MSG_BASE_ADDRESS 0x240

static SendCarCAN_Q_t CANFifo; 

static OS_SEM CarCAN_Sem4;
static OS_MUTEX CarCAN_Mtx;

// Counter array to reduce the frequency at which we send out messages
static uint8_t motorMsgCount[NUM_MOTOR_MSGS] = {0};
static uint8_t motorMsgThreshold[NUM_MOTOR_MSGS] = {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5};

static void Task_PutIOState(void *p_arg);

/**
 * @brief print SendCarCAN_Q put/get for debug purposes
*/
void print_SendCarCAN_Q(void) {
    if(SendCarCAN_Q_is_full(&CANFifo)) {
        printf("\n\rSendCarCAN_Q is full.");
    }
    printf("\n\rSendCarCAN_Q put: %d\n\rSendCarCAN_Q get: %d", CANFifo.put, CANFifo.get);
}

/**
 * @brief Wrapper to put new message in the CAN queue
*/
void SendCarCAN_Put(CANDATA_t message){
    OS_ERR err;
    CPU_TS ticks;

    // Reduce the frequency at which we forward motor messages to CarCAN
    uint8_t msgIdx = message.ID - MOTOR_MSG_BASE_ADDRESS;

    if (msgIdx >= 0 && msgIdx < NUM_MOTOR_MSGS) {  // Check if the message is from the motor controller
        if (++motorMsgCount[msgIdx] < motorMsgThreshold[msgIdx]) {
            return;  
        } else {
            motorMsgCount[msgIdx] = 0; // Reset counter and continue sending
        }     
    }
    
    OSMutexPend(&CarCAN_Mtx, 0, OS_OPT_PEND_BLOCKING, &ticks, &err);
    assertOSError(err);
    
    bool success = SendCarCAN_Q_put(&CANFifo, message);

    OSMutexPost(&CarCAN_Mtx, OS_OPT_POST_NONE, &err);
    assertOSError(err);

    if(success) OSSemPost(&CarCAN_Sem4, OS_OPT_POST_1, &err);
    assertOSError(err);
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
    memset(&message, 0, sizeof(CANDATA_t));

    // PutIOState
    OSTaskCreate(
        (OS_TCB*)&putIOState_TCB,
        (CPU_CHAR*)"PutIOState",
        (OS_TASK_PTR)Task_PutIOState,
        (void*)NULL,
        (OS_PRIO)TASK_SEND_CAR_CAN_PRIO,
        (CPU_STK*)putIOState_Stk,
        (CPU_STK_SIZE)WATERMARK_STACK_LIMIT,
        (CPU_STK_SIZE)TASK_SEND_CAR_CAN_STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)0,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    );
    assertOSError(err);

    while (1) {
          
        // Check if there's something to send in the queue (either IOState or Car state from sendTritium)
        OSSemPend(&CarCAN_Sem4, 0, OS_OPT_PEND_BLOCKING, &ticks, &err);
        assertOSError(err);

        OSMutexPend(&CarCAN_Mtx, 0, OS_OPT_PEND_BLOCKING, &ticks, &err);
        assertOSError(err);
    
        bool res = SendCarCAN_Q_get(&CANFifo, &message);
        assertOSError(err);

        OSMutexPost(&CarCAN_Mtx, OS_OPT_POST_NONE, &err);
        assertOSError(err);

        if(res) CANbus_Send(message, true, CARCAN);
        
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

    CANbus_Send(message, true, CARCAN);
}

/**
 * @brief sends IO information over CarCAN every IO_STATE_DLY_MS
*/
static void Task_PutIOState(void *p_arg) {
    OS_ERR err;
    while (1) {
        putIOState();
        OSTimeDlyHMSM(0, 0, 0, IO_STATE_DLY_MS, OS_OPT_TIME_HMSM_STRICT, &err);
    }
   
}
