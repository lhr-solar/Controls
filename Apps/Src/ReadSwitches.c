#include "Tasks.h"

// Needs to Read the Switches states (Driver level switches code) and trigger appropriate RTOS events
// Needs to Signal:
// VelocityChange_Sem4 : Counting Semaphore
// DisplayChange_Sem4 : Counting Semaphore
// LightsChange_Sem4 : Counting Semaphore
// CarCAN_Sem4 : Counting Semaphore
//
// ActivateArray_Sem4 : Binary Semaphore : IGN_1
// ActivateMotor_Sem4 : Binary Semaphore : IGN_2
// BlinkLight_Sem4  : Binary Semaphore

/**
 * @brief Reads Switches using Switches Driver and signals appropriate
 * Semaphores when Switches state changes.
*/
void Task_ReadSwitches (void* p_arg) {
    Switches_Init();
    while(1){
       if (Switches_Read(IGN_2)) {
           //Should post to both ActivateMotor and ActivateArray
       } else if (Switches_Read(IGN_1)) {
           //Should post to only ActivateArray and not ActivateMotor
       }
       //Check other switches
    }
}