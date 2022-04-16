/* Copyright (c) 2020 UT Longhorn Racing Solar */

#include "ReadCarCAN.h"
#include "Contactors.h"


static OS_ERR err;
static OS_MUTEX CANWatchdogMutex;
static bool msg_recieved = false;
static int watchDogTripCounter = 0; //count how many times the CAN watchdog trips

//CAN watchdog thread variables
static OS_TCB cwatchTCB;
static CPU_STK cwatchSTK;

//Array restart thread variables
static OS_TCB arrayTCB;
static CPU_STK arraySTK;


static void CANWatchdog_Handler(); //Handler if we stop getting messages
static void ArrayRestart(); //handler to turn array back on

void Task_ReadCarCAN(void *p_arg)
{
    uint8_t buffer[8]; // buffer for CAN message
    uint32_t canId;
    CPU_TS ts;
    
    OSMutexCreate( //create the mutex
        &CANWatchdogMutex,
        "CAN Watchdog message Mutex",
        &err
    );
    assertOSError(OS_READ_CAN_LOC,err);

    //Create+start the Can watchdog thread
    OSTaskCreate(
        &cwatchTCB,
        "CAN Watchdog",
        &CANWatchdog_Handler,
        NULL,
        4,
        &cwatchSTK,
        128/10,
        128,
        NULL,
        NULL,
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

            OSMutexPend(&CANWatchdogMutex,5,OS_OPT_PEND_BLOCKING,&ts, &err); //get mutex
            msg_recieved = true; //signal success recieved
            OSMutexPost(&CANWatchdogMutex,OS_OPT_POST_NONE,&err); //release the mutex
            assertOSError(OS_READ_CAN_LOC,err);

            if(buffer[0] != 1){ // If the buffer doesn't contain 1 for enable, turn off RegenEnable and turn array off
                RegenAllowed = OFF;
                //kill array restart thread 
                OSTaskDel(&arrayTCB,&err);
                //kill contactors 
                Contactors_Set(ARRAY_CONTACTOR, OFF); 
                Contactors_Set(ARRAY_PRECHARGE, OFF);
                assertOSError(OS_READ_CAN_LOC,err);
                continue;
            }

            //We got a message of enable with a nonzero value, turn on Regen, If we are already in precharge / array is on, do nothing. 
            //If not initiate precharge and restart sequence. 
            RegenAllowed = ON;
            if((Contactors_Get(ARRAY_CONTACTOR)==OFF) && (Contactors_Get(ARRAY_PRECHARGE)==OFF)){ // IF the both array contactors are off (nothing is precharging)
                Contactors_Set(ARRAY_PRECHARGE, ON); //turn on precharge contactor
                //Turn on the array restart thread
                OSTaskCreate(
                    &arrayTCB,
                    "Array Restarter",
                    &ArrayRestart,
                    NULL,
                    4,
                    &arraySTK,
                    128/10,
                    128,
                    NULL,
                    NULL,
                    NULL,
                    OS_OPT_TASK_STK_CLR,
                    &err
                );
                assertOSError(OS_READ_CAN_LOC,err); 
            }            
        }
        assertOSError(OS_READ_CAN_LOC,err);
    }
}

/**
 * @brief This function is the handler thread for the CANWatchdog timer. It disconnects the array and disables regenerative braking if we do not get
 * a CAN message with the ID Charge_Enable within the desired interval.
*/
static void CANWatchdog_Handler(){
    CPU_TS ts;
    while(1){
        OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_HMSM_STRICT,&err);
        assertOSError(OS_READ_CAN_LOC,err);
        OSMutexPend(&CANWatchdogMutex,5,OS_OPT_PEND_BLOCKING,&ts, &err);
        assertOSError(OS_READ_CAN_LOC,err);
        if(msg_recieved==true){
            msg_recieved = false;
            continue;
        } else {
            //kill the precharge thread
            OSTaskDel(&arrayTCB,&err);
            //turn off contactors
            Contactors_Set(ARRAY_CONTACTOR,OFF); //Turn off the contactors
            Contactors_Set(ARRAY_PRECHARGE, OFF);
            //increment trip counter
            watchDogTripCounter += 1;
        }
        OSMutexPost(&CANWatchdogMutex,OS_OPT_POST_NONE,&err); //release the mutex
        assertOSError(OS_READ_CAN_LOC,err);
    }

};

/**
 *  * @brief This function is the array precharge thread that gets instantiated when array restart needs to happen.
*/
static void ArrayRestart(){
    OSTimeDlyHMSM(0,0,0,PRECHARGE_ARRAY_DELAY*100,OS_OPT_TIME_HMSM_STRICT,&err); //delay
    assertOSError(OS_READ_CAN_LOC,err);
    Contactors_Set(ARRAY_CONTACTOR, ON); //turn on contactor and turn off precharge
    Contactors_Set(ARRAY_PRECHARGE, OFF);
};