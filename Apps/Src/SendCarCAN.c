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


// Make sure updated to the CarData_t and carMSGID are reflected in the CAN Bus IDs excel sheet

#define CARDATA_ID 0x581

typedef struct CarData{
    uint8_t accelPedal;
    uint8_t brakePedal;
    uint8_t minionBitmap;
    uint8_t contactorBitmap;
    uint32_t reserved;
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
    OS_ERR err;

    Minion_Error_t Merr;

    while (1) {
        // Send motor msg
        CAN_Queue_Pend(&motorMsg);
        CANbus_Send(motorMsg.id, motorMsg.payload, CAN_BLOCKING);

        // Get pedal information
        data.accelPedal = Pedals_Read(ACCELERATOR);
        data.brakePedal = Pedals_Read(BRAKE);

        // Get minion information
        data.minionBitmap = 0;
        for(MinionPin_t pin = 0; pin < MINIONPIN_NUM; pin++){
            bool pinState = Minion_Read_Input(pin, &Merr);
            data.minionBitmap |= pinState << pin;
        }

        // Get contactor info
        data.contactorBitmap = 0;
        for(contactor_t contactor = 0; contactor < NUM_CONTACTORS; contactor++){
            bool contactorState = Contactors_Get(contactor) == ON ? true : false;
            data.contactorBitmap |= contactorState << contactor;
        }

        // Send car msg
        carMsg.payload.data.d = *((uint64_t*)&data);
        CANbus_Send(CARDATA_ID, carMsg.payload, CAN_BLOCKING);

        // Delay of few milliseconds (100)
        OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &err);
        if (err != OS_ERR_NONE){
            assertOSError(OS_UPDATE_VEL_LOC, err);
        }
    }
}
