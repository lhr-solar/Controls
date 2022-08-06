/* Copyright (c) 2020 UT Longhorn Racing Solar */

#include "ReadCarCAN.h"
#include "Display.h"
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
    CANDATA_t message;
    message.ID = ARRAY_CONTACTOR_STATE_CHANGE;
    message.idxEn = false;
    message.idx = 0;
    message.size = 1;
    message.data = false;
    CAN_Queue_Post(message);

    // turn off the array contactor light
    Lights_Set(A_CNCTR, OFF);
    //Display_SetLight(A_CNCTR, OFF);
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

    // start precharge for array 
    if(shouldRestartArray){

         if (Contactors_Set(ARRAY_PRECHARGE, ON) == SUCCESS) {    // only run through precharge sequence if we successfully start precharge
            restartingArray = true;
            
            //Turn on the array restart thread
            OSTaskCreate(
                &arrayTCB,
                "Array Restarter",
                &ArrayRestart,
                NULL,
                TASK_READ_CAR_CAN_PRIO,
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

    OSMutexPost(&arrayRestartMutex,
                OS_OPT_NONE,
                &err);
    assertOSError(OS_READ_CAN_LOC,err);
}

void Task_ReadCarCAN(void *p_arg)
{
    OS_ERR err;

    //data struct for CAN message
    CANDATA_t dataBuf;
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
        TASK_READ_CAR_CAN_PRIO,
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
        ErrorStatus status = CANbus_Read(&dataBuf,CAN_BLOCKING,CARCAN);  
        if(status != SUCCESS) {
            continue;
        }

        switch(dataBuf.ID){ //we got a message
            case CHARGE_ENABLE: {
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

                if((&dataBuf.data)[0] == 0){ // If the buffer doesn't contain 1 for enable, turn off RegenEnable and turn array off
                    chargingDisable();
                } else {
                    //We got a message of enable with a nonzero value, turn on Regen, If we are already in precharge / array is on, do nothing. 
                    //If not initiate precharge and restart sequence. 
                    chargingEnable();
                }
                break;
            }
            case SUPPLEMENTAL_VOLTAGE: {
                SupplementalVoltage = *(uint16_t *) &(dataBuf.data);
                break;
            }
            case STATE_OF_CHARGE:{
                StateOfCharge = *(uint32_t*) &(dataBuf.data); //get the 32 bit message and store it
                break;
            }
            default: 
                break;
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
            chargingDisable();
            OSMutexPost(&msg_rcv_mutex,
                        OS_OPT_NONE,
                        &err);
            assertOSError(OS_READ_CAN_LOC,err);

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
    CPU_TS ts;

    OSTimeDlyHMSM(0,0,PRECHARGE_ARRAY_DELAY,0,OS_OPT_TIME_HMSM_STRICT,&err); //delay
    assertOSError(OS_READ_CAN_LOC,err);

    if(!RegenEnable){    // If regen enable has been disabled during precharge, we don't want to turn on the main contactor immediately after
        OSMutexPend(&arrayRestartMutex,
                0,
                OS_OPT_PEND_BLOCKING,
                &ts,
                &err);
        restartingArray = false;
        OSMutexPost(&arrayRestartMutex,
                OS_OPT_NONE,
                &err);
        assertOSError(OS_READ_CAN_LOC,err);
        return;
    }

    Contactors_Set(ARRAY_CONTACTOR, ON);
    Contactors_Set(ARRAY_PRECHARGE, OFF);
    Lights_Set(A_CNCTR, ON);
    //Display_SetLight(A_CNCTR, ON);

    // let array know the contactor is on
    CANDATA_t databuf;
    databuf.ID = ARRAY_CONTACTOR_STATE_CHANGE;
    databuf.size = 1;
    databuf.data = true;
    databuf.idxEn = false;
    databuf.idx = 0;
    CAN_Queue_Post(databuf);

    OSMutexPend(&arrayRestartMutex,
                0,
                OS_OPT_PEND_BLOCKING,
                &ts,
                &err);
    assertOSError(OS_READ_CAN_LOC,err);

    // done restarting the array
    restartingArray = false;

    OSMutexPost(&arrayRestartMutex,
                OS_OPT_NONE,
                &err);
    assertOSError(OS_READ_CAN_LOC,err);

    // delete this task
    OSTaskDel(&arrayTCB, &err);
};
