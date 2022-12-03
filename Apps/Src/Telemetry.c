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
 * Gathers the information of the pedals, lights, switches, and contactors 
 * to be read by telemetry
 * 
 */


// Make sure updated to the CarData_t and carMSGID are reflected in the CAN Bus IDs excel sheet

#define CARDATA_ID 0x581

// typedef struct CarData{
//     uint8_t accelPedal;
//     uint8_t brakePedal;
//     uint8_t minionBitmap;
//     uint8_t contactorBitmap;
//     uint32_t reserved;
// }CarData_t;

/**
 * @brief Sends pedal, switch, light, and contactor information to be read by telemetry
 * 
 * @param p_arg 
 */
void Task_Telemetry(void *p_arg){
    CANDATA_t carMsg;
    // CarData_t data;
    // data.reserved = 0;
    carMsg.ID = ODOMETER_AMPHOURS;  // ID is wrong
    OS_ERR err;

    Minion_Error_t Merr;

    while (1) {
        // Get pedal information
        carMsg.data[0] = Pedals_Read(ACCELERATOR);
        carMsg.data[1] = Pedals_Read(BRAKE);

        // Get minion information
        carMsg.data[2] = 0;
        for(MinionPin_t pin = 0; pin < MINIONPIN_NUM; pin++){
            bool pinState = Minion_Read_Input(pin, &Merr);
            carMsg.data[2] |= pinState << pin;
        }

        // Get contactor info
        carMsg.data[3] = 0;
        for(contactor_t contactor = 0; contactor < NUM_CONTACTORS; contactor++){
            bool contactorState = Contactors_Get(contactor) == ON ? true : false;
            carMsg.data[3] |= contactorState << contactor;
        }

        // Send car msg
        // carMsg.data[0] = *((uint64_t*)&data);
        CANbus_Send(carMsg, CAN_BLOCKING, CARCAN);

        // Delay of few milliseconds (100)
        OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &err);
        if (err != OS_ERR_NONE){
            assertOSError(OS_UPDATE_VEL_LOC, err);
        }
    }
}
