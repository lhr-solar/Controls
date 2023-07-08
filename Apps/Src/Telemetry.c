#include "Tasks.h"
#include "CANbus.h"
#include "CAN_Queue.h"
#include "Pedals.h"
#include "Minions.h"
#include "Contactors.h"
#include "common.h"


// Make sure updated to the CarData_t and carMSGID are reflected in the CAN Bus IDs excel sheet

/**
 * @brief Sends pedal, switch, light, and contactor information, as well as the 
 * feedback from the motor controller to be read by telemetry
 * 
 * @param p_arg 
 */
void Task_Telemetry(void *p_arg){
    CANDATA_t carMsg;
    carMsg.ID = CARDATA;  // ID is wrong
    for(int i = 4; i < 8; i++){
        carMsg.data[i] = 0;
    }
    OS_ERR err;
    CANDATA_t motorMsg;
    Minion_Error_t Merr;

    while (1) {

        // Send motor msg
        CAN_Queue_Pend(&motorMsg);
        CANbus_Send(motorMsg, true, CARCAN);

        // Get pedal information
        carMsg.data[0] = Pedals_Read(ACCELERATOR);
        carMsg.data[1] = Pedals_Read(BRAKE);

        // Get minion information
        carMsg.data[2] = 0;
        for(MinionPin_t pin = 0; pin < NUM_MINIONPINS; pin++){
            bool pinState = Minion_Read_Pin(pin, &Merr);
            carMsg.data[2] |= pinState << pin;
        }

        // Get contactor info
        carMsg.data[3] = 0;
        for(contactor_t contactor = 0; contactor < NUM_CONTACTORS; contactor++){
            bool contactorState = Contactors_Get(contactor) == ON ? true : false;
            carMsg.data[3] |= contactorState << contactor;
        }

        // Send car msg
        CANbus_Send(carMsg, true, CARCAN);

        // Delay of few milliseconds (500)
        OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &err);
        if (err != OS_ERR_NONE){
            assertOSError(OS_UPDATE_VEL_LOC, err);
        }
    }
}
