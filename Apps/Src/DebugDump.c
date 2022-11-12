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
//contactors
#define FOREACH_CONTACTOR(CONTACTOR) \
        CONTACTOR(ARRAY_CONTACTOR)   \
        CONTACTOR(ARRAY_PRECHARGE)  \
        CONTACTOR(MOTOR_CONTACTOR)   \
        CONTACTOR(NUM_CONTACTORS)  \

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

enum CONTACTOR_ENUM {
    FOREACH_CONTACTOR(GENERATE_ENUM)
};

static const char *CONTACTOR_STRING[] = {
    FOREACH_CONTACTOR(GENERATE_STRING)
};


//minion switches
#define FOREACH_MSWITCH(MSWITCH) \
        MSWITCH(IGN_1)   \
        MSWITCH(IGN_2)  \
        MSWITCH(REGEN_SW)   \
        MSWITCH(FOR_SW)  \
        MSWITCH(REV_SW)  \
        MSWITCH(REGEN_SW)   \
        MSWITCH(CRUZ_EN)  \
        MSWITCH(CRUZ_ST)  \

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

enum MSWITCH_ENUM {
    FOREACH_MSWITCH(GENERATE_ENUM)
};

static const char *MSWITCH_STRING[] = {
    FOREACH_MSWITCH(GENERATE_STRING)
};









void Task_Contactor_Ignition(void* p_arg) {
    OS_ERR err;
    Minion_Error_t mErr;


    // Get pedal information
    int8_t accelPedal = Pedals_Read(ACCELERATOR);
    char* testStr = "ACCELERATOR: %d", accelPedal;
    BSP_UART_Write(UART_3, testStr , strlen(testStr));

    int8_t brakePedal = Pedals_Read(BRAKE);
    char* testStr = "BRAKE: %d", brakePedal;
    BSP_UART_Write(UART_3, testStr , strlen(testStr));


    // Get switch information
    for(MinionPin_t swtch = 0; swtch < MINIONPIN_NUM; swtch++){
        bool switchState = Minion_Read_Input(swtch, &mErr) == ON ? true : false;
        char* testStr = "%s: %d      Error: %s",uhh,switchState,mErr;
        BSP_UART_Write(UART_3, (char*) testStr , strlen(testStr));
    }



    // Get light information
    bool light_info = (bool)Minion_Read_Input(BRAKELIGHT, &mErr);//WRONG BUT HOW ELSE DO U DO IT
    char* testStr = "BRAKELIGHT: %d      Error: %c",light_info,mErr;
    BSP_UART_Write(UART_3, (char*) testStr , strlen(testStr));


    // Get contactor info   
    for(contactor_t contactor = 0; contactor < NUM_CONTACTORS; contactor++){
        bool switchState = Contactors_Get(contactor) == ON ? true : false;
        char* testStr = "%s: %d", CONTACTOR_SRING[contactor], switch_state;
        BSP_UART_Write(UART_3, testStr , strlen(testStr));
    } 

}