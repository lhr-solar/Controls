#include "os.h"
#include "Tasks.h"
#include "bsp.h"
#include "CANbus.h"
#include "CAN_Queue.h"
#include "Pedals.h"
#include "Minions.h"
#include "Contactors.h"
#include "common.h"
#include <errno.h> 
#include "Tasks.h"

// global variables
extern fault_bitmap_t FaultBitmap;
extern os_error_loc_t OSErrLocBitmap;
extern State RegenEnable;
extern bool UpdateVel_ToggleCruise;
extern uint16_t SupplementalVoltage;
extern uint32_t StateOfCharge;

static const char *MINIONPIN_STRING[] = {
    FOREACH_MinionPin(GENERATE_STRING)
};

static const char *CONTACTOR_STRING[] = {
    FOREACH_contactor(GENERATE_STRING)
};

// Need to keep this in sync with Task.h
/*----------------------------------------------*/
#define FAULT_BITMAP_NUM 6
#define OS_LOC_NUM 14
static const char *OS_LOC_STRING[] = { 
    "OS_NONE_LOC", 
    "OS_ARRAY_LOC", 
    "OS_READ_CAN_LOC", 
    "OS_READ_TRITIUM_LOC",
    "OS_SEND_CAN_LOC",
    "OS_SEND_TRITIUM_LOC",
    "OS_UPDATE_VEL_LOC",
    "OS_CONTACTOR_LOC",
    "OS_MINIONS_LOC",
    "OS_MAIN_LOC",
    "OS_CANDRIVER_LOC",
    "OS_MOTOR_CONNECTION_LOC",
    "OS_DISPLAY_LOC"
};

static const char *FAULT_BITMAP_STRING[] = { 
    "FAULT_NONE", 
    "FAULT_OS", 
    "FAULT_UNREACH", 
    "FAULT_TRITIUM",
    "FAULT_READBPS",
    "FAULT_DISPLAY",
    "FAULT_BPS"
};
/*----------------------------------------------*/

void Task_DebugDump(void* p_arg) {
    OS_ERR err;
    Minion_Error_t mErr;

    while(1){

        // Get pedal information
        int8_t accelPedal = Pedals_Read(ACCELERATOR);
        printf("ACCELERATOR: %d\n\r", accelPedal);

        int8_t brakePedal = Pedals_Read(BRAKE);
        printf("BRAKE: %d\n\r", brakePedal);

        // Get minion information
        for(MinionPin_t pin = 0; pin < MINIONPIN_NUM-1; pin++){ // Plan to change MINIONPIN_NUM-1 -> MINIONPIN_NUM after Minion function is fixed
            bool pinState = Minion_Read_Pin(pin, &mErr);
            printf("%s: %s\n\r", MINIONPIN_STRING[pin], pinState == true ? "true" : "false");
        }

        // Get contactor info
        for(contactor_t contactor = 0; contactor < CONTACTOR_NUM; contactor++){
            bool contactorState = Contactors_Get(contactor) == ON ? true : false;
            printf("%s: %s\n\r", CONTACTOR_STRING[contactor], contactorState == true ? "true" : "false");
        } 

        // fault bitmap
        printf("Fault Bitmap: ");
        if(FaultBitmap == FAULT_NONE){
            printf("%s", FAULT_BITMAP_STRING[0]);
        }else{
            for(int i = 0; i < FAULT_BITMAP_NUM; i++){
                if(FaultBitmap & (1 << i)){
                    printf("%s ", FAULT_BITMAP_STRING[i]);
                }
            }
        }
        printf("\n\r");

        // os loc bitmap
        printf("OS Lock Bitmap: ");
        if(OSErrLocBitmap == OS_NONE_LOC){
            printf("%s", OS_LOC_STRING[0]);
        }
        else{
            for(int i = 0; i < OS_LOC_NUM; i++){
                if(OSErrLocBitmap & (1 << i)){
                    printf("%s ", OS_LOC_STRING[i]);
                }
            }
        }
        printf("\n\r");

        // update velocity toggle cruise
        printf("UpdateVel_ToggleCruise: %s\n\r", UpdateVel_ToggleCruise == true ? "True" : "False");

        // supplemental voltage
        printf("Supplemental Voltage: %d\n\r", SupplementalVoltage);

        // state of charge
        printf("State of Charge: %ld\n\r", StateOfCharge);

        // Delay of 5 seconds
        OSTimeDlyHMSM(0, 0, 5, 0, OS_OPT_TIME_HMSM_STRICT, &err);
        if (err != OS_ERR_NONE){
            assertOSError(OS_NONE_LOC, err);
        }
    }
}