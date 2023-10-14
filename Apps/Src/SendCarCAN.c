#include "common.h"
#include "os_cfg_app.h"

#include "Tasks.h"
#include "SendCarCAN.h"

#include "CANbus.h"
#include "Minions.h"
#include "Contactors.h"
#include "Pedals.h"
#include "Tasks.h"
#include "SendCarCAN.h"

// fifo
#define FIFO_TYPE CANDATA_t
#define FIFO_SIZE 256
#define FIFO_NAME SendCarCAN_Q
#include "fifo.h"

#define SENDCARCAN_QUEUE_CHECK_MS 50 // How often we should check the CAN queue (in ms delay). Must send faster than queue messages get put in
static SendCarCAN_Q_t CANFifo;

static OS_SEM CarCAN_Sem4;
static OS_MUTEX CarCAN_Mtx;
static OS_SEM PutIOTimerAlert_Sem4;

#define IO_STATE_TMR_DLY_MS 250u
#define IO_STATE_TMR_DLY_TS ((IO_STATE_TMR_DLY_MS * OS_CFG_TMR_TASK_RATE_HZ) / (1000u))
static OS_TMR IOStateTmr;

static void putIOStateTimerAlert(void *p_tmr, void *p_arg);
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

    OSSemCreate(&PutIOTimerAlert_Sem4, "PutIOTimerAlert_Sem4", 0, &err);
    assertOSError(OS_SEND_CAN_LOC, err);

    SendCarCAN_Q_renew(&CANFifo);
    
    // Set up timer to put car data message every IO_STATE_TMR_DLY_MS ms
    OSTmrCreate(&IOStateTmr,
                "IO State CAN Message Timer",
                0,
                IO_STATE_TMR_DLY_TS,
                OS_OPT_TMR_PERIODIC,
                putIOStateTimerAlert,
                NULL,
                &err
                );
    assertOSError(OS_SEND_CAN_LOC, err);

    OSTmrStart(&IOStateTmr, &err);
    assertOSError(OS_SEND_CAN_LOC, err);

    CANDATA_t message;
    memset(&message, 0, sizeof(CANDATA_t));

    while (1) {
        
        OSSemPend(&PutIOTimerAlert_Sem4, 0, OS_OPT_PEND_NON_BLOCKING, &ticks, &err);
        if (err != OS_ERR_PEND_WOULD_BLOCK) { // The timer fired; it's time to send an IO state message
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
            
        OSTimeDlyHMSM(0, 0, 0, SENDCARCAN_QUEUE_CHECK_MS, OS_OPT_TIME_HMSM_STRICT, &err);
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

// Callback function for the putIOState timer
static void putIOStateTimerAlert(void *p_tmr, void *p_arg){
    OS_ERR err;
    OSSemPost(&PutIOTimerAlert_Sem4, OS_OPT_POST_1, &err);
    assertOSError(OS_SEND_CAN_LOC, err);
}