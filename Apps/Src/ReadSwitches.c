//#include "Tasks.h"
#include <os.h>
#include "Switches.h"
#include "Tasks.h"

// Needs to Read the Switches states (Driver level switches code) and trigger appropriate RTOS events
// Needs to Signal:
// VelocityChange_Sem4 : Counting Semaphore : DONE
// DisplayChange_Sem4 : Counting Semaphore
// LightsChange_Sem4 : Counting Semaphore
// CarCAN_Sem4 : Counting Semaphore
//
// ActivateArray_Sem4 : Binary Semaphore : IGN_1 : done
// ActivateMotor_Sem4 : Binary Semaphore : IGN_2 : done
// BlinkLight_Sem4  : Binary Semaphore : LEFT_SW + RIGHT_SW : done




/**
 * @brief Reads Switches using Switches Driver and signals appropriate
 * Semaphores when Switches state changes.
*/
void Task_ReadSwitches (void* p_arg) {
    Switches_Init();
    OS_ERR err;
    while(1){

        //ActivateArray_SEM4 + ActivateMotor_SEM4
        uint8_t ignStates = 0x00; //holds states read from Switches Driver
        uint8_t ignStored = 0x00; //holds states stored in globals
        ignStates = ignStates || (Switches_Read(IGN_2)<<1);
        ignStates = ignStates || (Switches_Read(IGN_1));
        ignStored = ignStored || ((switches.IGN_2&0x01)<<1);
        ignStored = ignStored || (switches.IGN_1&0x01);
        if (ignStored!=ignStates){
            switch(ignStates){
                case 0x00:
                    //neither is on
                    switches.IGN_1 = 0;
                    switches.IGN_2 = 0;
                    //TODO: add car turn off code?
                case 0x01:
                //IGN1 is on
                    switches.IGN_1=1;
                    switches.IGN_2=0;
                    OSSemPost(
                        &ActivateArray_Sem4,
                        (OS_OPT) OS_OPT_POST_ALL,
                        (OS_ERR*)&err
                    );
                case 0x03:
                //Both are on
                    switches.IGN_2=1;
                    switches.IGN_1=1;
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
                case 0x02:
                    return -1; //Shouldn't happen - error case; only IGN2 is on
                
            }
       
        }
        
        //BlinkLight_Sem4 + LightsChange_Sem4
        switches_t lightSwitches[]={LEFT_SW,RIGHT_SW,HEADLIGHT_SW}; //the three switches that we need to check
        uint8_t storeLightSwitches[]={switches.LEFT_SW,switches.RIGHT_SW,switches.HEADLIGHT_SW}; //three global switch states we need
        uint8_t lightSwitchStates = 0x00; //holds states read from Switches_Driver
        uint8_t lightSwitchStored = 0x00; //holds states stored in global
        for(uint8_t i = 0; i<3; i++){
            lightSwitchStates = lightSwitchStates || (Switches_Read(lightSwitches[i])<<i);
            lightSwitchStored = lightSwitchStored || (storeLightSwitches[i]<<i);
        }
        if(lightSwitchStored!=lightSwitchStates){
            //if any of the bits dont match, LightsChange must get signaled. If the Blinker bits dont match blinker sem must ALSO get signaled
            //left sw: bit 0, right sw: bit 1, head sw: bit 2
            for(uint8_t i = 0; i<3; i++){
                storeLightSwitches[i]=(lightSwitchStates&(1<<i)); //store read values of states in buffer
            }

            switches.LEFT_SW = storeLightSwitches[0];
            switches.RIGHT_SW = storeLightSwitches[1];
            switches.HEADLIGHT_SW = storeLightSwitches[2];

            if((lightSwitchStored&0xFB)!=(lightSwitchStates&0xFB)){
                //Headlight bit was cleared, and they were still not equal -> blinkers states are new and blinklight has to be signaled
                OSSemPost(
                    &BlinkLight_Sem4,
                    (OS_OPT) OS_OPT_POST_ALL,
                    (OS_ERR*)&err
                );
            }
            OSSemPost(
                &LightsChange_Sem4,
                (OS_OPT) OS_OPT_POST_ALL,
                (OS_ERR*)&err
            );
        }

        //VelocityChange_Sem4
        switches_t velSwitches[] = {CRUZ_SW, CRUZ_EN, FR_SW, REV_SW, REGEN_SW};
        uint8_t storeVelSwitches[] = {switches.CRUZ_SW,switches.CRUZ_EN,switches.FR_SW,switches.REV_SW,switches.REGEN_SW};
        uint8_t velStates = 0x00; //holds read states of switches in least sig 5 bits
        uint8_t velStored = 0x00; //hold stored global states of velocity switches in least sig bits
        for(uint8_t i = 0; i<5;i++){
            velStates = velStates || (Switches_Read(velSwitches[i])<<i);
            velStored = velStored || ((storeVelSwitches[i]&1)<<i);
        }
        if(velStates!=velStored){
            //stored states and read states dont match up
            for(uint8_t i = 0; i<5; i++){
                storeVelSwitches[i]=(velStates&(1<<i));
            }
            switches.CRUZ_SW = storeVelSwitches[0];
            switches.CRUZ_EN=storeVelSwitches[1];
            switches.FR_SW=storeVelSwitches[2];
            switches.REV_SW=storeVelSwitches[3];
            switches.REGEN_SW=storeVelSwitches[4];
            OSSemPost(
                &VelocityChange_Sem4,
                (OS_OPT) OS_OPT_POST_ALL,
                (OS_ERR*)&err
            );
        }




        return 0;
    }
}