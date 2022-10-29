/* Copyright (c) 2020 UT Longhorn Racing Solar */

#include "ReadCarCAN.h"
#include "Display.h"
#include "Contactors.h"
#include "Minions.h"
#include "CAN_Queue.h"
#include "os_tmr.c"

/*
static bool msg_recieved = false;
static OS_MUTEX msg_rcv_mutex;
*/

// watchDogTripCounter variable and limit
static int CANWatchTripCounter = 0; 
static const int CANWATCH_TRIP_LIMIT = 2;

/*
//XXXCAN watchdog thread variables
static OS_TCB canWatchTCB;
static CPU_STK canWatchSTK[DEFAULT_STACK_SIZE];
*/

//MMM CAN watchdog timer variables
static OS_TMR canWatchTimer; // Is it ok to declare this here?
static int canWatchTmrDlySeconds = 5;

//MMM prechargeDlyTimer
static OS_TMR prechargeDlyTimer;

//MMM restart array and disable charging semaphonres
static OS_SEM restartArraySem;
static OS_SEM canWatchDisableChargingSem;

/*
//Array restart thread variables
static OS_TCB arrayTCB;
static CPU_STK arraySTK[DEFAULT_STACK_SIZE];
*/


// Array restart thread lock
static OS_MUTEX arrayRestartMutex;
static bool restartingArray = false;

/*
static void CANWatchdog_Handler(); //Handler if we stop getting messages */
static void arrayRestart(); //handler to turn array back on


// Need to add function prototypes here

// helper function to call if charging should be disabled
static inline void chargingDisable(void) {
    // mark regen as disabled
    RegenEnable = OFF;

    //kill contactors 
    Contactors_Set(ARRAY_CONTACTOR, OFF);
    Contactors_Set(ARRAY_PRECHARGE, OFF);
    
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

            // Wait to make sure precharge is finished and then restart array
            OSTmrStart(&prechargeDlyTimer, &err);
            
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
    uint8_t buffer[8]; // buffer for CAN message
    uint32_t canId;
    CPU_TS ts;

    OSMutexCreate(&arrayRestartMutex, "array restart mutex", &err);
    assertOSError(OS_READ_CAN_LOC,err);
    OSMutexCreate(&msg_rcv_mutex, "message received mutex", &err);
    assertOSError(OS_READ_CAN_LOC,err);
    OSSemCreate(&restartArraySem, "array restart semaphore", 0, &err);
    assertOSError(OS_READ_CAN_LOC, err);
    OSSemCreate(&canWatchDisableChargingSem, "disable charging semaphore", 0, &err);


    //Create the CAN Watchdog (periodic) timer
    /**
     * @brief This function is the handler thread for the CANWatchdog timer. It disconnects the array and disables regenerative braking if we do not get
     * a CAN message with the ID Charge_Enable within the desired interval.
    */
    OSTmrCreate(
        &canWatchTimer,
        "CAN Watch Timer",
        0,
        OS_CFG_TMR_TASK_RATE_HZ * canWatchTmrDlySeconds,
        OS_OPT_TMR_PERIODIC,
        &canWatchTimerCallback,
        NULL,
        &err
    );
    assertOSError(OS_READ_CAN_LOC, err);

    OSTmrCreate(
        &prechargeDlyTimer,
        "Precharge Delay Timer",
        OS_CFG_TMR_TASK_RATE_HZ * PRECHARGE_ARRAY_DELAY,
        0,
        OS_OPT_TMR_ONE_SHOT,
        &arrayRestart,
        NULL,
        &err
    );
    assertOSError(OS_READ_CAN_LOC, err);

    //Start CAN Watchdog timer
    OSTmrStart(&canWatchTimer, &err);


    /**/
    //Create+start the Can watchdog thread XXX will be replaced with timer
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
    */
    

    while (1)
    {

        
        //Get any message that BPS Sent us
        ErrorStatus status = CANbus_Read(&canId, buffer, CAN_BLOCKING); 
        if(status != SUCCESS) {
            continue;
        }

        switch(canId){ //we got a message
            case CHARGE_ENABLE: { //M Don't think we need msg_received
                /*OSMutexPend(&msg_rcv_mutex,
                    0,
                    OS_OPT_PEND_BLOCKING,
                    &ts,
                    &err);
                assertOSError(OS_READ_CAN_LOC,err);

                msg_recieved = true; //signal success recieved //M Replace with restarting canWatchtimer

                OSMutexPost(&msg_rcv_mutex,
                            OS_OPT_NONE,
                            &err);
                assertOSError(OS_READ_CAN_LOC,err); */
                //Start CAN Watchdog timer
                OSTmrStart(&canWatchTimer, &err);

                if(buffer[0] == 0){ // If the buffer doesn't contain 1 for enable, turn off RegenEnable and turn array off
                    chargingDisable();
                } else {
                    //We got a message of enable with a nonzero value, turn on Regen, If we are already in precharge / array is on, do nothing. 
                    //If not initiate precharge and restart sequence. 
                    chargingEnable();
                }
                break;
            }
            case SUPPLEMENTAL_VOLTAGE: {
                SupplementalVoltage = *(uint16_t *) &buffer;
                break;
            }
            case STATE_OF_CHARGE:{
                StateOfCharge = *(uint32_t*) &buffer; //get the 32 bit message and store it
                break;
            }
            default: 
                break;
        }
    }
}



/**
 * @brief Disables charging and increments the trip counter
 * when canWatchTimer hits 0 (when charge enable messages are missed)
 * 
*/
void canWatchTimerCallback (){  //Probably needs its timer arguments
    // mark regen as disabled
    RegenEnable = OFF;

    //increment trip counter
    CANWatchTripCounter++;
    
    // Set fault bitmaps 
    FaultBitmap |= FAULT_READBPS;
    OSErrLocBitmap |= OS_READ_CAN_LOC;

    // If we tripped the counter too many times, enter fault state
    if(CANWatchTripCounter > CANWATCH_TRIP_LIMIT) {
        EnterFaultState();
    } else { // Otherwise signal fault state to kill contactors and return
        OSSemPost(&FaultState_Sem4, OS_OPT_POST_1, &err);
    }
}

/**
 * @brief restart the array. 
*/
static void arrayRestart(void *p_arg){
    OS_ERR err;
    CPU_TS ts;

    if(!RegenEnable){    // If regen enable has been disabled during precharge, we don't want to turn on the main contactor immediately after
        restartingArray = false;
    }
    else {
        if (contactors[ARRAY_CONTACTOR].enabled) {
            setContactor(ARRAY_CONTACTOR, ON);
        }
        if (contactors[ARRAY_PRECHARGE].enabled) {
            setContactor(ARRAY_PRECHARGE, OFF); //Is this whole just trying to turn off precharge thing ok? Maybe I should...um...
        }
        /*
        Contactors_Set(ARRAY_CONTACTOR, ON);
        Contactors_Set(ARRAY_PRECHARGE, OFF);
        */
        Lights_Set(A_CNCTR, ON);

        //Display_SetLight(A_CNCTR, ON);

       /* // let array know the contactor is on
        CANMSG_t msg;
        msg.id = ARRAY_CONTACTOR_STATE_CHANGE;
        msg.payload.bytes = 1;
        msg.payload.data.b = true;
        CAN_Queue_Post(msg);
        */

        // done restarting the array
        restartingArray = false;
    }
};



