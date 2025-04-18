#include "Tasks.h"
#include "CANbus.h"
#include "daybreak_pins.h"
#include "StatusLeds.h"
#include "CANConfig.h"


#define TEST_CARCAN_LOOPBACK

int main(){
    Status_Leds_Init();

    Status_Leds_Write(OS_FAULT_LED, true);

    #ifdef TEST_CARCAN_LOOPBACK
    CANbus_Init(CARCAN, (CANId_t *) carCANFilterList, NUM_CARCAN_FILTERS);
    #elif TEST_MOTORCAN_lOOPBACK
    CANbus_Init(MOTORCAN, (CANId_t *) motorCANFilterList, NUM_MOTORCAN_FILTERS);
    #endif

    CANDATA_t msg, out;    
    CAN_t bus;     
    msg.idx = 0;
    memset(&msg.data, 0xa5, sizeof msg.data);


    #ifdef TEST_MOTORCAN_LOOPBACK
    bus = MOTORCAN;
    msg.ID = VELOCITY;         
    #elif defined(TEST_CARCAN_LOOPBACK)
    bus = CARCAN;
    msg.ID = BPS_TRIP;
    #endif

    while(1){
        ErrorStatus sendError = CANbus_Send(msg, true, bus);
        Status_Leds_Write(CONTROLS_FAULT_LED, sendError == SUCCESS ? ON : OFF);
        ErrorStatus readError = CANbus_Read(&out, true, bus);
        Status_Leds_Write(BPS_FAULT_LED, readError == SUCCESS ? ON : OFF);
        Status_Leds_Write(MOTOR_CONTROLLER_FAULT_LED, ON);
    }
}