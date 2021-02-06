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
        uint8_t ignstates = 0x00;
        ignstates = ignstates || (Switches_Read(IGN_2)<<1);
        ignstates = ignstates || (Switches_Read(IGN_1));
        switch(ignstates){
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
       
        //BlinkLight_Sem4
        uint8_t blinkStates = 0x00;
        blinkStates = blinkStates || (Switches_Read(LEFT_SW)<<1);
        blinkStates = blinkStates || (Switches_Read(RIGHT_SW));       
        switch(blinkStates){
            case 0x03:
            //both are on
                switches.LEFT_SW = 1;
                switches.RIGHT_SW = 1;
                OSSemPost(
                    &BlinkLight_Sem4,
                    (OS_OPT) OS_OPT_POST_ALL,
                    (OS_ERR*)&err
                );
            case 0x02:
            //Left Switch is on
                switches.LEFT_SW = 1;
                switches.RIGHT_SW = 0;
                OSSemPost(
                    &BlinkLight_Sem4,
                    (OS_OPT) OS_OPT_POST_ALL,
                    (OS_ERR*)&err
                );
            case 0x01:
            //Right Switch is on
                switches.LEFT_SW = 0;
                switches.RIGHT_SW = 1;
                OSSemPost(
                    &BlinkLight_Sem4,
                     (OS_OPT) OS_OPT_POST_ALL,
                    (OS_ERR*)&err
                );
            case 0x00:
            //neither are on
                switches.LEFT_SW = 0;
                switches.RIGHT_SW = 0;
            // OSSemSet(
            //     &BlinkLight_Sem4,
            //     (OS_SEM_CTR) 0,
            //     (OS_ERR*)&err
            // );
        }

        //VelocityChange_Sem4
        switches_t velSwitches[] = {CRUZ_SW, CRUZ_EN, FR_SW, REV_SW, REGEN_SW};
        uint8_t storeSwitches[] = {switches.CRUZ_SW,switches.CRUZ_EN,switches.FR_SW,switches.REV_SW,switches.REGEN_SW};
        uint8_t velReadStates = 0x00; //holds read states of switches in least sig 5 bits
        uint8_t velStoreStates = 0x00; //hold stored global states of velocity switches in least sig bits
        for(int i = 0; i<5;i++){
            velReadStates = velReadStates || (Switches_Read(velSwitches[i])<<i);
            velStoreStates = velStoreStates || (storeSwitches[i]<<i);
        }
        if(velReadStates!=velStoreStates){
            //stored states and read states dont match up
            for(int i = 0; i<5; i++){
                storeSwitches[i]=(velReadStates&(1<<i));
            }
            switches.CRUZ_SW = storeSwitches[0];
            switches.CRUZ_EN=storeSwitches[1];
            switches.FR_SW=storeSwitches[2];
            switches.REV_SW=storeSwitches[3];
            switches.REGEN_SW=storeSwitches[4];
            OSSemPost(
                &VelocityChange_Sem4,
                (OS_OPT) OS_OPT_POST_ALL,
                (OS_ERR*)&err
            );
        }

        //LightsChange_SEM4
        


        return 0;
    }
}