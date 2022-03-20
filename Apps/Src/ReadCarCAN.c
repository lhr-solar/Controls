/* Copyright (c) 2020 UT Longhorn Racing Solar */

#include "ReadCarCAN.h"
#include "Contactors.h"


static OS_TMR CANWatchdog; //watchdog timer to trigger fault if we stop getting messages
static OS_TMR ArrayRestartTimer; //Timer to restart the array properly after precharge.
static OS_ERR err;
static bool RESTART_TRIGGER = false; //Flag to indicate whether we are in precharge or not
static int watchDogTripCounter = 0; //count how many times the CAN watchdog trips

static void CANWatchdog_Handler(); //Handler if we stop getting messages
static void ArrayRestart(); //handler to turn array back on

void Task_ReadCarCAN(void *p_arg)
{
    uint8_t buffer[8]; // buffer for CAN message
    uint32_t canId;

    OSTmrCreate( //Create Can watchdog 
        (OS_TMR*) &CANWatchdog,
        (CPU_CHAR*) "CAN Watchdog Timer",
        (OS_TICK)50, //Our tick rate is set to 100Hz in OS_CFG_APP.h. 100 HZ = 10ms period -> 50 ticks = 500ms
        (OS_TICK)0,
        (OS_OPT) OS_OPT_TMR_ONE_SHOT,
        (OS_TMR_CALLBACK_PTR) &CANWatchdog_Handler,
        (void*) NULL,
        (OS_ERR*) &err
    );
    assertOSError(0,err);

    OSTmrCreate( //create array precharge timer for re-enabling charge 
        (OS_TMR*) &ArrayRestartTimer,
        (CPU_CHAR*) "Array Restart sequence",
        (OS_TICK)PRECHARGE_ARRAY_DELAY*100, //Our tick rate is set to 100Hz in OS_CFG_APP.h. 100 HZ = 10ms period -> (Precharge_Array_Delay * 1000)/10 = delay in ticks
        (OS_TICK)0,
        (OS_OPT) OS_OPT_TMR_ONE_SHOT,
        (OS_TMR_CALLBACK_PTR) &ArrayRestart,
        (void*) NULL,
        (OS_ERR*) &err
    );
    assertOSError(0,err);

    OSTmrStart( //start the CAN message watchdog
        (OS_TMR*) &CANWatchdog,
        (OS_ERR*) &err
    );
    assertOSError(0,err);

    while (1)
    {
        //Get any message that BPS Sent us
        ErrorStatus status = CANbus_Read(&canId, buffer, CAN_BLOCKING);
        if(status == SUCCESS && canId == CHARGE_ENABLE){ //we got a charge_enable message
            OSTmrStart( //Pet the watchdog since we got a charge_Enable message
                (OS_TMR*) &CANWatchdog,
                (OS_ERR*) &err
            );
            assertOSError(OS_READ_CAN_LOC,err);
            if(!(buffer[0]==1)){ //If the buffer doesn't contain anything in the LSByte, turn off RegenEnable and array contactor off
                RegenAllowed = OFF;
                Contactors_Set(ARRAY_CONTACTOR, OFF);
                //kill restart if it is going on
                RESTART_TRIGGER = false;
                OSTmrStop(
                    (OS_TMR*) &ArrayRestartTimer,
                    (OS_OPT) OS_OPT_TMR_NONE,
                    (void*) NULL,
                    (OS_ERR*) &err
                );
                assertOSError(0,err);
                continue;
            }

            //We got a message of enable, turn on Regen, If we are already in precharge / array is on, do nothing. 
            //If not initiate precharge and restart sequence. 
            RegenAllowed = ON;
            if(!RESTART_TRIGGER){ //If restart hasn't been triggered, trigger it
                Contactors_Set(ARRAY_PRECHARGE, ON);
                RESTART_TRIGGER = true;
                OSTmrStart(
                    (OS_TMR*) &ArrayRestartTimer,
                    (OS_ERR*) &err
                );
                assertOSError(0,err); //TODO: add actual error location
            }
            
        }
    }
}

/**
 * @brief This function is the handler for the CANWatchdog timer. It disconnects the array and disables regenerative braking if we do not get
 * a CAN message with the ID Charge_Enable within the desired interval.
*/
static void CANWatchdog_Handler(){
    //TODO: Do we want this to signal the Global Fault Handler and kill the system? Or do we want to be able to recover from this
    Contactors_Set(ARRAY_CONTACTOR,OFF);
    RESTART_TRIGGER = false; //kill restart process if it is on
    OSTmrStop(
        (OS_TMR*) &ArrayRestartTimer,
        (OS_OPT) OS_OPT_TMR_NONE,
        (void*) NULL,
        (OS_ERR*) &err
    );
    assertOSError(0,err);
    watchDogTripCounter += 1;
};

/**
 * @brief This function is a callback that gets triggered to reconnect the array after the precharge timer hits zero.
*/
static void ArrayRestart(){
    Contactors_Set(ARRAY_CONTACTOR, ON);
    Contactors_Set(ARRAY_PRECHARGE, OFF);
    RESTART_TRIGGER = false; //restart process complete, reset trigger
};