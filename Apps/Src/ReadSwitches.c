//#include "Tasks.h"
#include <os.h>
#include "Switches.h"
#include "Tasks.h"
#include "CarState.h"


/**
 * @brief Reads Switches using Switches Driver and signals appropriate
 * Semaphores when Switches state changes. Also updates a switches global defined in StateTypes.h, declared in Tasks.c and externed in Tasks.h
*/
void Task_ReadSwitches (void* p_arg) {
    car_state_t *prevCarState= (car_state_t*)p_arg;
    switch_states_t *prevSwitchStates = &(prevCarState->SwitchStates); //pointer to global switchstates
    switch_states_t currSwitchStates; //holds states read from driver reads

    // load newSwitchStates
    currSwitchStates = (switch_states_t){
        Switches_Read(CRUZ_SW),
        Switches_Read(CRUZ_EN),
        Switches_Read(HZD_SW),
        Switches_Read(FWD_SW),
        Switches_Read(HEADLIGHT_SW),
        Switches_Read(LEFT_SW),
        Switches_Read(RIGHT_SW),
        Switches_Read(REGEN_SW),
        Switches_Read(IGN_1),
        Switches_Read(IGN_2),
        Switches_Read(REV_SW),
    };

    Switches_Init();
    OS_ERR err;
    static bool arrayConThreadSpawned = false; //these booleans need to be persistent
    static bool motorConThreadSpawned = false;
    while(1){
        //Spawns arrayConnection thread and MotorConnection thread when it is appropriate
        uint8_t ignStates = 0x00; //holds states read from Switches Driver
        uint8_t ignStored = 0x00; //holds states stored in globals
        ignStates|=((currSwitchStates.IGN_2&1)<<1); //shift IGN_2 over 1 and set it in ignStates
        ignStates|=(currSwitchStates.IGN_1&1);
        ignStored|=((prevSwitchStates->IGN_2&1)<<1);
        ignStored|=(prevSwitchStates->IGN_1&1);
        if (ignStored!=ignStates){
            switch(ignStates){
                case 0x00:
                    prevCarState->ShouldArrayBeActivated = 0;
                    prevCarState->ShouldMotorBeActivated = 0;
                    OSSemPost(
                        &ArrayConnectionChange_Sem4,
                        (OS_OPT) OS_OPT_POST_ALL,
                        (OS_ERR*)&err
                    );
                    OSSemPost(
                        &MotorConnectionChange_Sem4,
                        (OS_OPT) OS_OPT_POST_ALL,
                        (OS_ERR*)&err
                    );
                case 0x01:
                //IGN1 is on
                    prevCarState->ShouldArrayBeActivated = 1;
                    prevCarState->ShouldMotorBeActivated = 0;
                    if (arrayConThreadSpawned==false){ 
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
                        arrayConThreadSpawned=true;
                    }
                
                case 0x03:
                //Both are on
                    prevCarState->ShouldArrayBeActivated = 1;
                    prevCarState->ShouldMotorBeActivated = 1;
                    if(arrayConThreadSpawned==false){ 
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
                        arrayConThreadSpawned=true;
                    }
                    if(motorConThreadSpawned==false){
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
                        motorConThreadSpawned=true;
                    };
                case 0x02:
                    return -1; //Shouldn't happen - error case; only IGN2 is on
            }
       
        }
        
        //Signal LightsChange_Sem4 for UpdateLights Thread
        State curLightSwitches[]={
            currSwitchStates.LEFT_SW,
            currSwitchStates.RIGHT_SW,
            currSwitchStates.HEADLIGHT_SW,
            currSwitchStates.HZD_SW
        }; 
        State prevLightSwitches[]={
            prevSwitchStates->LEFT_SW,
            prevSwitchStates->RIGHT_SW,
            prevSwitchStates->HEADLIGHT_SW,
            prevSwitchStates->HZD_SW
        };       
        uint8_t lightSwitchStates = 0x00; //holds states read from Switches_Driver
        uint8_t lightSwitchStored = 0x00; //holds states stored in global
        for(uint8_t i = 0; i<3; i++){
            lightSwitchStates |= (curLightSwitches[i]<<i);
            lightSwitchStored |= (prevLightSwitches[i]<<i);
        }
        if(lightSwitchStored!=lightSwitchStates){
            //if any of the bits dont match, LightsChange must get signaled       
            OSSemPost(
                &LightsChange_Sem4,
                (OS_OPT) OS_OPT_POST_ALL,
                (OS_ERR*)&err
            );
        }

        //VelocityChange_Sem4
        State readVelSwitches[] = {
            currSwitchStates.CRUZ_SW,
            currSwitchStates.CRUZ_EN,
            currSwitchStates.FWD_SW,
            currSwitchStates.REV_SW,
            currSwitchStates.REGEN_SW
        };
        State storeVelSwitches[] = {
            prevSwitchStates->CRUZ_SW,
            prevSwitchStates->CRUZ_EN,
            prevSwitchStates->FWD_SW,
            prevSwitchStates->REV_SW,
            prevSwitchStates->REGEN_SW
        };
        uint8_t velStates = 0x00; //holds read states of switches in least sig 5 bits
        uint8_t velStored = 0x00; //hold stored global states of velocity switches in least sig bits
        for(uint8_t i = 0; i<5;i++){
            velStates |=((readVelSwitches[i]&1)<<i);
            velStored |=((storeVelSwitches[i]&1)<<i);
        }
        if(velStates!=velStored){
            //stored states and read states dont match up
            OSSemPost(
                &VelocityChange_Sem4,
                (OS_OPT) OS_OPT_POST_ALL,
                (OS_ERR*)&err
            );
        }

        //DisplayChange_Sem4
        State readDispSwitches[] = {
            currSwitchStates.CRUZ_EN,
            currSwitchStates.CRUZ_SW,
            currSwitchStates.REGEN_SW
        };
        State storeDispSwitches[] = {
            prevSwitchStates->CRUZ_EN,
            prevSwitchStates->CRUZ_SW,
            prevSwitchStates->REGEN_SW
        };
        uint8_t dispStates = 0x00;
        uint8_t dispStored = 0x00;
        for(uint8_t i=0;i<3;i++){
            dispStates |= ((readDispSwitches[i])<<i);
            dispStored |= ((storeDispSwitches[i])<<i);
        }
        if(dispStates != dispStored){ //if any of the states don't match up sem4 needs a signal
            OSSemPost(
                &DisplayChange_Sem4,
                (OS_OPT) OS_OPT_POST_ALL,
                (OS_ERR*)&err
            );
        }


        //set global state to match what was read
        *prevSwitchStates = currSwitchStates; 

    }
}