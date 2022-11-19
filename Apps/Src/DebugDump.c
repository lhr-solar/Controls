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

extern FaultBitmap;
extern OSErrLocBitmap;
extern SupplementalVoltage;
extern StateOfCharge;
extern switch_state;
extern RegenEnable;
extern msg_queue;
extern UpdateVel_ToggleCruise;




void Task_DebugDump(void* p_arg) {
    //OS_ERR err;
    Minion_Error_t mErr;


    // Get pedal information
    int8_t accelPedal = Pedals_Read(ACCELERATOR);
    printf("ACCELERATOR: %d", accelPedal);

    int8_t brakePedal = Pedals_Read(BRAKE);
    printf("BRAKE: %d", brakePedal);


    // Get minion switch information
    for(MinionPin_t swtch = 0; swtch < MINIONPIN_NUM; swtch++){
        bool switchState = Minion_Read_Input(swtch, &mErr) == true ? true : false;
        printf("%s: %d",MSWITCH_STRING[swtch],switchState);
    }


    // // Get minion light information(reading from an ouput-an error for this function 'Minion_Read_Input')
    // bool light_info = (bool)Minion_Read_Input(BRAKELIGHT, &mErr);
    // testStr = "BRAKELIGHT: %d",light_info;
    // BSP_UART_Write(UART_3, (char*) testStr , strlen(testStr));


    // Get contactor info   
    for(contactor_t contactor = 0; contactor < NUM_CONTACTORS; contactor++){
        bool switchState = Contactors_Get(contactor) == true ? true : false;
        printf("%s: %d", CONTACTOR_STRING[contactor], switch_state);
    } 


    //FaultState and errlocbitmap    
    printf("FaultBitmap: %s", FAULT_BITMAP_STRING[FaultBitmap]);  //*what is this

    printf("OSErrLocBitmap: %s", OS_LOC_STRING[OSErrLocBitmap]);  //*what is this

    printf("SupplementalVoltage: %d", SupplementalVoltage);  //*what is this

    printf("StateOfCharge: %d", StateOfCharge);  //*what is this



    // RegenEnable
    printf("RegenEnable: %d", RegenEnable);


    //UpdateVelocity Globals
    printf("UpdateVel_ToggleCruise: %d", UpdateVel_ToggleCruise);


    //UpdateDisplay Globals
    printf("msg_queue: %c", msg_queue);  //*what is this
}