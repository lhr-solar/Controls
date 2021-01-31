//#include "Tasks.h"
#include <os.h>
#include "Switches.h"
#include "Tasks.h"

// Needs to Read the Switches states (Driver level switches code) and trigger appropriate RTOS events
// Needs to Signal:
// VelocityChange_Sem4 : Counting Semaphore
// DisplayChange_Sem4 : Counting Semaphore
// LightsChange_Sem4 : Counting Semaphore
// CarCAN_Sem4 : Counting Semaphore
//
// ActivateArray_Sem4 : Binary Semaphore : IGN_1 : done
// ActivateMotor_Sem4 : Binary Semaphore : IGN_2 : done
// BlinkLight_Sem4  : Binary Semaphore

/**
 * @brief Reads Switches using Switches Driver and signals appropriate
 * Semaphores when Switches state changes.
*/
void Task_ReadSwitches (void* p_arg) {
    Switches_Init();
    OS_ERR err;
    while(1){

        //ignition check
        uint8_t ignstates = 0x00;
        ignstates = ignstates || (Switches_Read(IGN_2)<<1);
        ignstates = ignstates || (Switches_Read(IGN_1));
        switch(ignstates){
            //TODO: add 0b00 case where car turns off
            case 0b01:
            //IGN1 is on
            OSSemPost(
                &ActivateArray_Sem4,
                (OS_OPT) OS_OPT_POST_ALL,
                (OS_ERR*)&err
            );
            case 0b11:
            //Both are on
            OSSemPost(
               &ActivateMotor_Sem4,
               (OS_OPT) OS_OPT_POST_ALL,
               (OS_ERR*)&err
            );
            OSSemPost(
                &ActivateArray_Sem4,
                (OS_OPT) OS_OPT_POST_ALL,
                (OS_ERR*)&err
            );
            case 0b10:
                return -1; //Shouldn't happen - error case
            
        }
       
        //blinkers check
        if(Switches_Read(LEFT_SW)&&(!Switches_Read(RIGHT_SW))){
            switches.LEFT_SW = 1;
            switches.RIGHT_SW = 0;
            OSSemPost(
               &BlinkLight_Sem4,
               (OS_OPT) OS_OPT_POST_ALL,
               (OS_ERR*)&err
           );
        } else if (Switches_Read(RIGHT_SW)&&(!Switches_Read(LEFT_SW))){
            switches.LEFT_SW = 0;
            switches.RIGHT_SW = 1;
            OSSemPost(
               &BlinkLight_Sem4,
               (OS_OPT) OS_OPT_POST_ALL,
               (OS_ERR*)&err
           );
        } else if (Switches_Read(RIGHT_SW)&&Switches_Read(LEFT_SW)){
            switches.LEFT_SW = 1;
            switches.RIGHT_SW = 1;
            OSSemPost(
               &BlinkLight_Sem4,
               (OS_OPT) OS_OPT_POST_ALL,
               (OS_ERR*)&err
           );
        } else {
            switches.LEFT_SW = 0;
            switches.RIGHT_SW = 0;
        };



        return 0;
    }
}