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

extern fault_bitmap_t FaultBitmap;
extern os_error_loc_t OSErrLocBitmap;
extern uint16_t SupplementalVoltage;
extern uint32_t StateOfCharge;
extern bool switch_state;
extern State RegenEnable;
extern int msg_queue;
extern bool UpdateVel_ToggleCruise;
extern char* FAULT_BITMAP_STRING[];
extern char* OS_LOC_STRING[];




void Task_DebugDump(void* p_arg) {
    OS_ERR err;
    // Minion_Error_t mErr;

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

    //FaultState and errlocbitmap    
    printf("FaultBitmap: %s", FAULT_BITMAP_STRING[FaultBitmap]);  //*what is this

    printf("OSErrLocBitmap: %s", OS_LOC_STRING[OSErrLocBitmap]);  //*what is this

    printf("SupplementalVoltage: %d", SupplementalVoltage);  //*what is this

    printf("StateOfCharge: %ld", StateOfCharge);  //*what is this



    // RegenEnable
    printf("RegenEnable: %d", RegenEnable);


    //UpdateVelocity Globals
    printf("UpdateVel_ToggleCruise: %d", UpdateVel_ToggleCruise);


    //UpdateDisplay Globals
    printf("msg_queue: %d", msg_queue);  //*what is this

    // Delay of 5 seconds (100)
    OSTimeDlyHMSM(0, 0, 5, 0, OS_OPT_TIME_HMSM_STRICT, &err);
    if (err != OS_ERR_NONE){
        assertOSError(OS_UPDATE_VEL_LOC, err);
    }
}