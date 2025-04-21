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
#include "Ignition.h"
#include "Dashboard.h"
#include "StatusLeds.h"
#include "Tasks.h"
#include "SendCarCAN.h"
#include "SendTritium.h"

#define IO_STATE_DLY_MS 250u 
#define IO_STATE_HEARTBEAT_DELAY (1000u)/IO_STATE_DLY_MS

#define SENDCARCAN_MSG_SKIP_CTR 3

// Task_PutIOState
OS_TCB putIOState_TCB;
CPU_STK putIOState_Stk[TASK_SEND_CAR_CAN_STACK_SIZE];

//fifo
#define FIFO_TYPE CANDATA_t
#define FIFO_SIZE 50
#define FIFO_NAME SendCarCAN_Q
#include "fifo.h"

static SendCarCAN_Q_t CANFifo; 

static OS_SEM CarCAN_Sem4;
static OS_MUTEX CarCAN_Mtx;

static void Task_PutIOState(void *p_arg);

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

    // PutIOState
    OSTaskCreate(
        (OS_TCB*)&putIOState_TCB,
        (CPU_CHAR*)"PutIOState",
        (OS_TASK_PTR)Task_PutIOState,
        (void*)NULL,
        (OS_PRIO)TASK_PUT_IOSTATE_PRIO,
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

        OSMutexPost(&CarCAN_Mtx, OS_OPT_POST_NONE, &err);
        assertOSError(err);

        if(res) CANbus_Send(message, true, CARCAN);
    }
}

static void putIOState(void){
    CANDATA_t message;
    memset(&message, 0, sizeof message);
    message.ID = IO_STATE;

    // Get pedal information
    message.data[0] = Pedals_Read(ACCELERATOR);
    int8_t brake_pedal = Pedals_Read(BRAKE);
    message.data[1] = brake_pedal;

    // If the brake is pressed far enough, send the state to turn on the brakelight
    message.data[2] |= (brake_pedal >= PEDAL_BRAKELIGHT_THRESHOLD) ? 1:0;

    // Send Cruise states
    message.data[2] |= SWITCH_BITMAP_CRUZ_EN(getDashState(DASHBOARD_CRUZ_EN));
    message.data[2] |= SWITCH_BITMAP_CRUZ_ST(getDashState(DASHBOARD_CRUZ_SET));

    // Regen is always disabled for daybreak
    message.data[2] |= SWITCH_BITMAP_REGEN_SW(0);

    switch(getDashState(DASHBOARD_GEAR)){
        case FWD:
            message.data[2] |= SWITCH_BITMAP_FOR_SW(1);
            break;
        case REV:
            message.data[2] |= SWITCH_BITMAP_REV_SW(1);
            break;
        default:
            message.data[2] |= SWITCH_BITMAP_FOR_SW(0);
            message.data[2] |= SWITCH_BITMAP_REV_SW(0);
            break;
    }

    // Send ignition states
    switch(Get_Ignition_State()){
        case IGN_ARR:
            // array state comes after motor so both array on motor are considered on at array state
            message.data[2] |= SWITCH_BITMAP_IGN_1_ARRAY(1);
            message.data[2] |= SWITCH_BITMAP_IGN_2_MOTOR(1);
            break;
        case IGN_MOTOR:
            message.data[2] |= SWITCH_BITMAP_IGN_1_ARRAY(0);
            message.data[2] |= SWITCH_BITMAP_IGN_2_MOTOR(1);
            break;
        default:
            message.data[2] |= SWITCH_BITMAP_IGN_1_ARRAY(0);
            message.data[2] |= SWITCH_BITMAP_IGN_2_MOTOR(0);
            break;
    }

    CANbus_Send(message, true, CARCAN);
}

/**
 * @brief sends IO information over CarCAN every IO_STATE_DLY_MS
*/
static void Task_PutIOState(void *p_arg) {
    OS_ERR err;
    static uint8_t ioStateCounter = 0;
    while (1) {
        putIOState();
        ioStateCounter++;
        // toggle dashboard led every 1 second (IoState runs at 250ms)
        if(ioStateCounter >= IO_STATE_HEARTBEAT_DELAY){
            Status_Leds_Toggle(DASH_HEARTBEAT_LED); // heartbeat led on the dashboard
            ioStateCounter = 0;
        }
        OSTimeDlyHMSM(0, 0, 0, IO_STATE_DLY_MS, OS_OPT_TIME_HMSM_STRICT, &err);
        assertOSError(err);
    }  
}
