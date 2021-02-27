//#include "Tasks.h"
#include <os.h>
#include "Switches.h"
#include "Tasks.h"

/**
 * @brief Reads Switches using Switches Driver and signals appropriate
 * Semaphores when Switches state changes. Also updates a switches global defined in StateTypes.h, declared in Tasks.c and externed in Tasks.h
*/
void Task_ReadSwitches (void* p_arg) {
    Switches_Init();
    OS_ERR err;
    while(1){
        //Spawns arrayConnection thread and MotorConnection thread when it is appropriate
        uint8_t ignStates = 0x00; //holds states read from Switches Driver
        uint8_t ignStored = 0x00; //holds states stored in globals
        ignStates = ignStates || (Switches_Read(IGN_2)<<1);
        ignStates = ignStates || (Switches_Read(IGN_1));
        ignStored = ignStored || ((carState.SwitchStates.IGN_2&&0x01)<<1);
        ignStored = ignStored || (carState.SwitchStates.IGN_1&&0x01);
        if (ignStored!=ignStates){
            switch(ignStates){
                case 0x00:
                    //neither is on
                    carState.SwitchStates.IGN_1 = 0;
                    carState.SwitchStates.IGN_2 = 0;
                    //TODO: add car turn off code?
                case 0x01:
                //IGN1 is on
                    carState.SwitchStates.IGN_1=1;
                    carState.SwitchStates.IGN_2=0;
                    //OSTASK CREATE activate array
                    // OSSemPost(
                    //     &ActivateArray_Sem4,
                    //     (OS_OPT) OS_OPT_POST_ALL,
                    //     (OS_ERR*)&err
                    // );
                    OSTaskCreate(
                        (OS_TCB*)&ArrayConnection_TCB,
                        (CPU_CHAR*)"Array Connection",
                        (OS_TASK_PTR)Task_ArrayConnection,
                        (void*)NULL,
                        (OS_PRIO)10,
                        (CPU_STK*)ArrayConnection_Stk,
                        (CPU_STK_SIZE)WATERMARK_STACK_LIMIT,
                        (CPU_STK_SIZE)TASK_ARRAY_CONNECTION_STACK_SIZE,
                        (OS_MSG_QTY)NULL,
                        (OS_TICK)NULL,
                        (void*)NULL,
                        (OS_OPT)(OS_OPT_TASK_STK_CLR),
                        (OS_ERR*)&err
                    );
                case 0x03:
                //Both are on
                    carState.SwitchStates.IGN_2=1;
                    carState.SwitchStates.IGN_1=1;
                    //OSTASKCREATE activate array, OSTASKCREATE activate motor
                    OSTaskCreate(
                        (OS_TCB*)&ArrayConnection_TCB,
                        (CPU_CHAR*)"Array Connection",
                        (OS_TASK_PTR)Task_ArrayConnection,
                        (void*)NULL,
                        (OS_PRIO)TASK_ARRAY_CONNECTION_PRIO,
                        (CPU_STK*)ArrayConnection_Stk,
                        (CPU_STK_SIZE)WATERMARK_STACK_LIMIT,
                        (CPU_STK_SIZE)TASK_ARRAY_CONNECTION_STACK_SIZE,
                        (OS_MSG_QTY)NULL,
                        (OS_TICK)NULL,
                        (void*)NULL,
                        (OS_OPT)(OS_OPT_TASK_STK_CLR),
                        (OS_ERR*)&err
                    );

                    OSTaskCreate(
                        (OS_TCB*)&MotorConnection_TCB,
                        (CPU_CHAR*)"Motor Connection",
                        (OS_TASK_PTR)Task_MotorConnection,
                        (void*)NULL,
                        (OS_PRIO)TASK_MOTOR_CONNECTION_PRIO,
                        (CPU_STK*)ArrayConnection_Stk,
                        (CPU_STK_SIZE)WATERMARK_STACK_LIMIT,
                        (CPU_STK_SIZE)TASK_MOTOR_CONNECTION_STACK_SIZE,
                        (OS_MSG_QTY)NULL,
                        (OS_TICK)NULL,
                        (void*)NULL,
                        (OS_OPT)(OS_OPT_TASK_STK_CLR),
                        (OS_ERR*)&err
                    );
                case 0x02:
                    return -1; //Shouldn't happen - error case; only IGN2 is on
                
            }
       
        }
        
        //Signal LightsChange_Sem4 for UpdateLights Thread
        switches_t lightSwitches[]={LEFT_SW,RIGHT_SW,HEADLIGHT_SW}; //the three switches that we need to check
        State storeLightSwitches[]={carState.SwitchStates.LEFT_SW,carState.SwitchStates.RIGHT_SW,carState.SwitchStates.HEADLIGHT_SW}; //three global switch states we need
        uint8_t lightSwitchStates = 0x00; //holds states read from Switches_Driver
        uint8_t lightSwitchStored = 0x00; //holds states stored in global
        for(uint8_t i = 0; i<3; i++){
            lightSwitchStates = lightSwitchStates || (Switches_Read(lightSwitches[i])<<i);
            lightSwitchStored = lightSwitchStored || (storeLightSwitches[i]<<i);
        }
        if(lightSwitchStored!=lightSwitchStates){
            //if any of the bits dont match, LightsChange must get signaled. If the Blinker bits dont match blinker sem must ALSO get signaled
            //left sw: bit 0, right sw: bit 1, head sw: bit 2

            //set globals
            carState.SwitchStates.LEFT_SW = (lightSwitchStates&&1);
            carState.SwitchStates.RIGHT_SW = (lightSwitchStates&&2);
            carState.SwitchStates.HEADLIGHT_SW = (lightSwitchStates&&4);
            
            //post to lightschange
            OSSemPost(
                &LightsChange_Sem4,
                (OS_OPT) OS_OPT_POST_ALL,
                (OS_ERR*)&err
            );
        }

        //VelocityChange_Sem4
        switches_t velSwitches[] = {CRUZ_SW, CRUZ_EN, FR_SW, REV_SW, REGEN_SW};
        uint8_t storeVelSwitches[] = {carState.SwitchStates.CRUZ_SW,carState.SwitchStates.CRUZ_EN,carState.SwitchStates.FR_SW,carState.SwitchStates.REV_SW,carState.SwitchStates.REGEN_SW};
        uint8_t velStates = 0x00; //holds read states of switches in least sig 5 bits
        uint8_t velStored = 0x00; //hold stored global states of velocity switches in least sig bits
        for(uint8_t i = 0; i<5;i++){
            velStates = velStates || (Switches_Read(velSwitches[i])<<i);
            velStored = velStored || ((storeVelSwitches[i]&&1)<<i);
        }
        if(velStates!=velStored){
            //stored states and read states dont match up
            carState.SwitchStates.CRUZ_SW = (velStates&&1);
            carState.SwitchStates.CRUZ_EN=(velStates&&2);
            carState.SwitchStates.FR_SW=(velStates&&4);
            carState.SwitchStates.REV_SW=(velStates&&8);
            carState.SwitchStates.REGEN_SW=(velStates&&16);
            OSSemPost(
                &VelocityChange_Sem4,
                (OS_OPT) OS_OPT_POST_ALL,
                (OS_ERR*)&err
            );
        }

        //DisplayChange_Sem4
        switches_t dispSwitches[] = {CRUZ_EN,CRUZ_SW,REGEN_SW};
        uint8_t storeDispSwitches[] = {carState.SwitchStates.CRUZ_EN,carState.SwitchStates.CRUZ_SW,carState.SwitchStates.REGEN_SW};
        uint8_t dispStates = 0x00;
        uint8_t dispStored = 0x00;
        for(uint8_t i=0;i<3;i++){
            dispStates = dispStates || ((Switches_Read(dispSwitches[i]))<<i);
            dispStored = dispStored || ((storeDispSwitches[i])<<i);
        }
        if(dispStates != dispStored){
            carState.SwitchStates.CRUZ_EN = (dispStates&&1);
            carState.SwitchStates.CRUZ_SW = (dispStates&&2);
            carState.SwitchStates.REGEN_SW = (dispStates&&4);
            OSSemPost(
                &DisplayChange_Sem4,
                (OS_OPT) OS_OPT_POST_ALL,
                (OS_ERR*)&err
            );
        }


    }
}