/* Copyright (c) 2020 UT Longhorn Racing Solar */

#include "ReadCarCAN.h"
#include "Contactors.h"
#include "Minions.h"
#include "CAN_Queue.h"


static bool msg_recieved = false;
static OS_MUTEX msg_rcv_mutex;


static int watchDogTripCounter = 0; //count how many times the CAN watchdog trips

//CAN watchdog thread variables
static OS_TCB canWatchTCB;
static CPU_STK canWatchSTK[DEFAULT_STACK_SIZE];

//Array restart thread variables
static OS_TCB arrayTCB;
static CPU_STK arraySTK[DEFAULT_STACK_SIZE];

// Array restart thread lock
static OS_MUTEX arrayRestartMutex;
static bool restartingArray = false;

static void CANWatchdog_Handler(); //Handler if we stop getting messages
static void ArrayRestart(); //handler to turn array back on

// helper function to call if charging should be disabled
static inline void chargingDisable(void) {
    // mark regen as disabled
    RegenEnable = OFF;
    //kill contactors 
    Contactors_Set(ARRAY_CONTACTOR, OFF);
    Contactors_Set(ARRAY_PRECHARGE, OFF);

    // let array know we killed contactors
    CANMSG_t msg;
    msg.id = ARRAY_CONTACTOR_STATE_CHANGE;
    msg.payload.bytes = 1;
    msg.payload.data.b = false;
    CAN_Queue_Post(msg);

    // turn off the array contactor light
    Lights_Set(A_CNCTR, OFF);
}

// helper function to call if charging should be enabled
static inline void chargingEnable(void) {
    OS_ERR err;
    CPU_TS ts;

    // mark regen as enabled
    RegenEnable = ON;

    // check if we need to run the precharge sequence to turn on the array
    bool shouldRestartArray = false;

    OSMutexPend(&arrayRestartMutex,
                0,
                OS_OPT_PEND_BLOCKING,
                &ts,
                &err);
    assertOSError(OS_READ_CAN_LOC,err);

    // if nothing is precharging and we need to turn the array on, turn it on
    shouldRestartArray = !restartingArray && (Contactors_Get(ARRAY_CONTACTOR)==OFF) && (Contactors_Get(ARRAY_PRECHARGE)==OFF);

    // don't let anyone restart the array while we are restarting it
    if (shouldRestartArray) {
        restartingArray = true;
    }

    OSMutexPost(&arrayRestartMutex,
                OS_OPT_NONE,
                &err);
    assertOSError(OS_READ_CAN_LOC,err);


    // start precharge for array 
    if(shouldRestartArray){
        //Turn on the array restart thread
        OSTaskCreate(
            &arrayTCB,
            "Array Restarter",
            &ArrayRestart,
            NULL,
            4,
            arraySTK,
            DEFAULT_STACK_SIZE/10,
            DEFAULT_STACK_SIZE,
            0,
            1,
            NULL,
            OS_OPT_TASK_STK_CLR,
            &err
        );
        assertOSError(OS_READ_CAN_LOC,err);
    }

    
}

void Task_ReadCarCAN(void *p_arg)
{
    OS_ERR err;
    uint8_t buffer[8]; // buffer for CAN message
    uint32_t canId;
    CPU_TS ts;

    OSMutexCreate(&arrayRestartMutex, "array restart mutex", &err);
    assertOSError(OS_READ_CAN_LOC,err);
    OSMutexCreate(&msg_rcv_mutex, "message received mutex", &err);
    assertOSError(OS_READ_CAN_LOC,err);

    //Create+start the Can watchdog thread
    OSTaskCreate(
        &canWatchTCB,
        "CAN Watchdog",
        &CANWatchdog_Handler,
        NULL,
        4,
        canWatchSTK,
        DEFAULT_STACK_SIZE/10,
        DEFAULT_STACK_SIZE,
        0,
        0,
        NULL,
        OS_OPT_TASK_STK_CLR,
        &err
    );
    assertOSError(OS_READ_CAN_LOC,err);

    while (1)
    {
        //Get any message that BPS Sent us
        ErrorStatus status = CANbus_Read(&canId, buffer, CAN_NON_BLOCKING); 
        if(status == SUCCESS && canId == CHARGE_ENABLE){ //we got a charge_enable message

            OSMutexPend(&msg_rcv_mutex,
                0,
                OS_OPT_PEND_BLOCKING,
                &ts,
                &err);
            assertOSError(OS_READ_CAN_LOC,err);

            msg_recieved = true; //signal success recieved

            OSMutexPost(&msg_rcv_mutex,
                        OS_OPT_NONE,
                        &err);
            assertOSError(OS_READ_CAN_LOC,err);

            if(buffer[0] == 0){ // If the buffer doesn't contain 1 for enable, turn off RegenEnable and turn array off
                chargingDisable();
            } else {

                //We got a message of enable with a nonzero value, turn on Regen, If we are already in precharge / array is on, do nothing. 
                //If not initiate precharge and restart sequence. 
                chargingEnable();
            }
        }
    }
}

/**
 * @brief This function is the handler thread for the CANWatchdog timer. It disconnects the array and disables regenerative braking if we do not get
 * a CAN message with the ID Charge_Enable within the desired interval.
*/
static void CANWatchdog_Handler(void *p_arg){
    OS_ERR err;
    CPU_TS ts;

    while(1){
        OSTimeDlyHMSM(0,0,5,0,OS_OPT_TIME_HMSM_STRICT,&err);
        assertOSError(OS_READ_CAN_LOC,err);

        OSMutexPend(&msg_rcv_mutex,
                0,
                OS_OPT_PEND_BLOCKING,
                &ts,
                &err);
        assertOSError(OS_READ_CAN_LOC,err);
        if(msg_recieved==true){
            msg_recieved = false;
            OSMutexPost(&msg_rcv_mutex,
                        OS_OPT_NONE,
                        &err);
            assertOSError(OS_READ_CAN_LOC,err);
            
        } else {
            OSMutexPost(&msg_rcv_mutex,
                        OS_OPT_NONE,
                        &err);
            assertOSError(OS_READ_CAN_LOC,err);

            chargingDisable();
            //increment trip counter
            watchDogTripCounter += 1;
        }
    }

};

/**
 *  * @brief This function is the array precharge thread that gets instantiated when array restart needs to happen.
*/
static void ArrayRestart(void *p_arg){
    OS_ERR err;

    if (Contactors_Set(ARRAY_PRECHARGE, ON) == SUCCESS) {    // only run through precharge sequence if we successfully start precharge

        OSTimeDlyHMSM(0,0,PRECHARGE_ARRAY_DELAY,0,OS_OPT_TIME_HMSM_STRICT,&err); //delay
        assertOSError(OS_READ_CAN_LOC,err);

        Contactors_Set(ARRAY_CONTACTOR, ON);
        Contactors_Set(ARRAY_PRECHARGE, OFF);
        Lights_Set(A_CNCTR, ON);

        // let array know the contactor is on
        CANMSG_t msg;
        msg.id = ARRAY_CONTACTOR_STATE_CHANGE;
        msg.payload.bytes = 1;
        msg.payload.data.b = true;
        CAN_Queue_Post(msg);
    }

    // done restarting the array
    restartingArray = false;

    // delete this task
    OSTaskDel(&arrayTCB, &err);
};
