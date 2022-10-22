#include "Tasks.h"
#include "common.h"
#include "CANbus.h"
#include "CAN_Queue.h"
#include "Pedals.h"
#include "Minions.h"
#include "Contactors.h"

#define carMSGID 0x581

typedef union DATAMSG{
    struct{
        uint64_t accelPedal : 8;
        uint64_t brakePedal : 8;
        uint64_t switchBitmap : 16;
        uint64_t lightBitmap : 8;
        uint64_t contactorBitmap : 8;
        uint64_t reserved : 16;
    };
    uint64_t val;
}DATAMSG_t;

void Task_SendCarCAN(void *p_arg){
    CANMSG_t motorMSG;
    CANMSG_t carMSG;
    DATAMSG_t data;

    while (1) {
        // Send motor msg
        CAN_Queue_Pend(&motorMSG);
        CANbus_Send(motorMSG.id, motorMSG.payload, CAN_BLOCKING);

        // Get pedal information
        data.accelPedal = Pedals_Read(ACCELERATOR);
        data.brakePedal = Pedals_Read(BRAKE);

        // Get switch information
        for(int i = 0; i < NUM_SWITCHES; i++){
            bool switchState = (bool)Switches_Read(i);
            data.switchBitmap = data.switchBitmap | (switchState << i);
        }

        // Get light information
        data.lightBitmap = (bool)Lights_Read(BrakeLight);

        // Get contactor info
        for(int i = 0; i < NUM_CONTACTORS; i++){
            bool switchState = (bool)Contactors_Get(i);
            data.contactorBitmap = data.contactorBitmap | (switchState << i);
        }

        // Send car msg
        carMSG.payload.data.d = data.val;
        CANbus_Send(carMSGID, carMSG.payload, CAN_BLOCKING);
    }
    
}
