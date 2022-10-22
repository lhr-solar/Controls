#include "Tasks.h"
#include "CANbus.h"
#include "CAN_Queue.h"
#include "Pedals.h"
#include "Minions.h"
#include "Contactors.h"
#include "common.h"

// Make sure updated to the DATAMSG and carMSGID are reflected in the CAN Bus IDs excel sheet

#define CARDATA_ID 0x581

typedef union CarData{
    struct{
        uint64_t accelPedal : 8;
        uint64_t brakePedal : 8;
        uint64_t switchBitmap : 16;
        uint64_t lightBitmap : 8;
        uint64_t contactorBitmap : 8;
        uint64_t reserved : 16;
    };
    uint64_t val;
}CarData_t;

void Task_SendCarCAN(void *p_arg){
    CANMSG_t motorMsg;
    CANMSG_t carMsg;
    CarData_t data;

    while (1) {
        // Send motor msg
        CAN_Queue_Pend(&motorMsg);
        CANbus_Send(motorMsg.id, motorMsg.payload, CAN_BLOCKING);

        // Get pedal information
        data.accelPedal = Pedals_Read(ACCELERATOR);
        data.brakePedal = Pedals_Read(BRAKE);

        // Get switch information
        for(switches_t swtch = 0; swtch < NUM_SWITCHES; swtch++){
            bool switchState = Switches_Read(swtch) == ON ? true : false;
            data.switchBitmap = data.switchBitmap | (switchState << swtch);
        }

        // Get light information
        data.lightBitmap = (bool)Lights_Read(BrakeLight);

        // Get contactor info
        for(contactor_t contactor = 0; contactor < NUM_CONTACTORS; contactor++){
            bool switchState = Contactors_Get(contactor) == ON ? true : false;
            data.contactorBitmap = data.contactorBitmap | (switchState << contactor);
        }

        // Send car msg
        carMsg.payload.data.d = data.val;
        CANbus_Send(CARDATA_ID, carMsg.payload, CAN_BLOCKING);
    }
    
}
