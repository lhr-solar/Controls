/* Copyright (c) 2020 UT Longhorn Racing Solar */

#include "ReadCarCAN.h"
#include "Contactors.h"


static OS_TMR CANWatchdog; //watchdog timer to trigger fault if we stop getting messages
static OS_TMR ArrayRestartTimer; //Timer to restart the array properly after precharge.
static OS_ERR err;
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
        ErrorStatus status = CANbus_Read(&canId, buffer, CAN_BLOCKING); //NOTE: This function acts as an OS Scheduling point because it contains a sempend
        if(status == SUCCESS && canId == CHARGE_ENABLE){ //we got a charge_enable message
            OSTmrStart( //Pet the watchdog since we got a charge_Enable message
                (OS_TMR*) &CANWatchdog,
                (OS_ERR*) &err
            );
            assertOSError(OS_READ_CAN_LOC,err);
            if(!(buffer[0]==1)){ //If the buffer doesn't contain anything in the LSByte, turn off RegenEnable and array off
                RegenAllowed = OFF;
                Contactors_Set(ARRAY_CONTACTOR, OFF); //kill contactors and the array restart timer
                Contactors_Set(ARRAY_PRECHARGE, OFF);
                OSTmrStop(
                    (OS_TMR*) &ArrayRestartTimer,
                    (OS_OPT) OS_OPT_TMR_NONE,
                    (void*) NULL,
                    (OS_ERR*) &err
                );
                assertOSError(OS_READ_CAN_LOC,err);
                continue;
            }

            //We got a message of enable, turn on Regen, If we are already in precharge / array is on, do nothing. 
            //If not initiate precharge and restart sequence. 
            RegenAllowed = ON;
            if((Contactors_Get(ARRAY_CONTACTOR)==OFF) && (Contactors_Get(ARRAY_PRECHARGE)==OFF)){ // IF the array is off and we are not already in precharge sequence.
                Contactors_Set(ARRAY_PRECHARGE, ON); //turn on precharge sequence
                OSTmrStart(
                    (OS_TMR*) &ArrayRestartTimer,
                    (OS_ERR*) &err
                );
                assertOSError(OS_READ_CAN_LOC,err); 
            }            
        }
        assertOSError(OS_READ_CAN_LOC,err);
    }
}

/**
 * @brief This function is the handler for the CANWatchdog timer. It disconnects the array and disables regenerative braking if we do not get
 * a CAN message with the ID Charge_Enable within the desired interval.
*/
static void CANWatchdog_Handler(){
    Contactors_Set(ARRAY_CONTACTOR,OFF); //Kill array and precharge sequence
    Contactors_Set(ARRAY_PRECHARGE, OFF);
    OSTmrStop(
        (OS_TMR*) &ArrayRestartTimer,
        (OS_OPT) OS_OPT_TMR_NONE,
        (void*) NULL,
        (OS_ERR*) &err
    );
    assertOSError(OS_READ_CAN_LOC,err);
    watchDogTripCounter += 1;
};

/**
 * @brief This function is a callback that gets triggered to reconnect the array after the precharge timer hits zero.
*/
static void ArrayRestart(){
    Contactors_Set(ARRAY_CONTACTOR, ON); //turn on contactor and precharge
    Contactors_Set(ARRAY_PRECHARGE, OFF);
};