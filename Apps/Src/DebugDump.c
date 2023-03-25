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

#define FAULT_BITMAP_NUM 6
#define OS_LOC_NUM 14

extern fault_bitmap_t FaultBitmap;
extern os_error_loc_t OSErrLocBitmap;
extern State RegenEnable;
extern bool UpdateVel_ToggleCruise;
// extern disp_fifo_t msg_queue;

static const char *MINIONPIN_STRING[] = {
    FOREACH_MinionPin(GENERATE_STRING)
};

static const char *CONTACTOR_STRING[] = {
    FOREACH_contactor(GENERATE_STRING)
};

// Need to keep this in sync with Task.h
static const char *OS_LOC_STRING[] = { 
    "OS_NONE_LOC", 
    "OS_ARRAY_LOC", 
    "OS_READ_CAN_LOC", 
    "OS_READ_TRITIUM_LOC",
    "OS_SEND_CAN_LOC",
    "OS_SEND_TRITIUM_LOC",
    "OS_UPDATE_VEL_LOC",
    "OS_BLINK_LIGHTS_LOC",
    "OS_CONTACTOR_LOC",
    "OS_MINIONS_LOC",
    "OS_MAIN_LOC",
    "OS_CANDRIVER_LOC",
    "OS_MOTOR_CONNECTION_LOC",
    "OS_DISPLAY_LOC"
};

// Need to keep this in sync with Task.h
static const char *FAULT_BITMAP_STRING[] = { 
    "FAULT_NONE", 
    "FAULT_OS", 
    "FAULT_UNREACH", 
    "FAULT_TRITIUM",
    "FAULT_READBPS",
    "FAULT_DISPLAY"
};

void Task_DebugDump(void* p_arg) {
    OS_ERR err;
    Minion_Error_t mErr;

    // Get pedal information
    int8_t accelPedal = Pedals_Read(ACCELERATOR);
    printf("ACCELERATOR: %d\n", accelPedal);

    int8_t brakePedal = Pedals_Read(BRAKE);
    printf("BRAKE: %d\n", brakePedal);

    // Get minion information
    for(MinionPin_t pin = 0; pin < MINIONPIN_NUM-1; pin++){ // Plan to change MINIONPIN_NUM-1 -> MINIONPIN_NUM after Minion function is fixed
        bool pinState = Minion_Read_Input(pin, &mErr);
        printf("%s: %d\n", MINIONPIN_STRING[pin], pinState);
    }

    // Get contactor info
    for(contactor_t contactor = 0; contactor < CONTACTOR_NUM; contactor++){
        bool contactorState = Contactors_Get(contactor) == ON ? true : false;
        printf("%s: %d\n", CONTACTOR_STRING[contactor], contactorState);
    } 

    // fault bitmap
    printf("Fault Bitmap: ");
    if(FaultBitmap == 0){
        printf("%s\n", FAULT_BITMAP_STRING[0]);
    }
    else{
        for(int i = 0; i < FAULT_BITMAP_NUM; i++){
            uint32_t result = (FaultBitmap & 1 << i) >> i;
            if(result){
                printf("%s ", FAULT_BITMAP_STRING[i]);
            }
        }
    }
    printf("\n");

    // os loc bitmap
    printf("Os Loc Bitmap: ");
    if(OSErrLocBitmap == 0){
        printf("%s\n", OS_LOC_STRING[0]);
    }
    else{
        for(int i = 0; i < OS_LOC_NUM; i++){
            uint32_t result = (OSErrLocBitmap & 1 << i) >> i;
            if(result){
                printf("%s ", OS_LOC_STRING[i]);
            }
        }
    }
    printf("\n");

    // regen enable
    // printf("RegenEnable: %s\n", RegenEnable == ON ? "On" : "Off");

    // update velocity toggle cruise
    printf("UpdateVel_ToggleCruise: %s\n", UpdateVel_ToggleCruise == true ? "True" : "False");

    // // display fifo
    // printf("msg_queue: %d", msg_queue);

    // Delay of 5 seconds
    OSTimeDlyHMSM(0, 0, 5, 0, OS_OPT_TIME_HMSM_STRICT, &err);
    if (err != OS_ERR_NONE){
        assertOSError(OS_NONE_LOC, err);
    }
}