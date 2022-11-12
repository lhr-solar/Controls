#include "Tasks.h"
#include "CANbus.h"
#include "CAN_Queue.h"
#include "Pedals.h"
#include "Minions.h"
#include "Contactors.h"
#include "common.h"


/**
 * @file SendCarCAN.c
 * @brief Implements the SendCarCAN Task
 * 
 * Creates a datatype to house the data to be read by telemetry
 * 
 * Resends the feedback from the motor controller and gathers the information 
 * of the pedals, lights, switches, and contactors to be read by telemetry
 * 
 */


// Make sure updated to the DATAMSG and carMSGID are reflected in the CAN Bus IDs excel sheet

#define CARDATA_ID 0x581

typedef union CarData{
    struct{
        uint64_t accelPedal : 8;
        uint64_t brakePedal : 8;
        uint64_t minionBitmap : 8;
        uint64_t contactorBitmap : 8;
        uint64_t reserved : 32;
    };
    uint64_t val;
}CarData_t;


/**
 * @brief Sends the feedback from the motor controller to be read by telemetry and 
 * sends pedal, switch, light, and contactor information to be read by telemetry
 * 
 * @param p_arg 
 */
void Task_SendCarCAN(void *p_arg){
    CANMSG_t motorMsg;
    CANMSG_t carMsg;
    CarData_t data;

    Minion_Error_t Merr;

    while (1) {
        // Send motor msg
        CAN_Queue_Pend(&motorMsg);
        CANbus_Send(motorMsg.id, motorMsg.payload, CAN_BLOCKING);

        //Initialize CarData & Minion Err
        Merr = MINION_ERR_NONE;
        data.val = 0;

        // Get pedal information
        data.accelPedal = Pedals_Read(ACCELERATOR);
        data.brakePedal = Pedals_Read(BRAKE);

        // Get minion information
        for(MinionPin_t pin = 0; pin < MINIONPIN_NUM; pin++){
            bool pinState = Minion_Read_Input(pin, &Merr);
            data.minionBitmap |= pinState << pin;
        }

        // Get contactor info
        for(contactor_t contactor = 0; contactor < NUM_CONTACTORS; contactor++){
            bool contactorState = Contactors_Get(contactor) == ON ? true : false;
            data.contactorBitmap |= contactorState << contactor;
        }

        // Send car msg
        carMsg.payload.data.d = data.val;
        CANbus_Send(CARDATA_ID, carMsg.payload, CAN_BLOCKING);
    }
    
}
