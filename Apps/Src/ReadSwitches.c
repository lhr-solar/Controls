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
    //Get Global States
    car_state_t *CarState= (car_state_t*)p_arg;
    switch_states_t *globalSwitchStates = &(CarState->SwitchStates);
    switch_states_t currSwitchStates; //holds states read from driver reads
    Switches_Init();

    
    OS_ERR err;
    static bool arrayConThreadSpawned = false; //these booleans need to be persistent
    static bool motorConThreadSpawned = false;

    while(1){
        // load newSwitchStates
        currSwitchStates = (switch_states_t){
            Switches_Read(IGN_1),
            Switches_Read(IGN_2),
            (velocity_switches_t){
                Switches_Read(CRUZ_SW),
                Switches_Read(CRUZ_EN),
                Switches_Read(FWD_SW),
                Switches_Read(REV_SW),
                Switches_Read(REGEN_SW)
            },
            (light_switches_t){
                Switches_Read(LEFT_SW),
                Switches_Read(RIGHT_SW),
                Switches_Read(HEADLIGHT_SW),
                Switches_Read(HZD_SW)
            }
        };
        
        //Spawns arrayConnection thread and MotorConnection thread when it is appropriate
        uint8_t ignStates = 0x00; //holds states read from Switches Driver
        uint8_t ignStored = 0x00; //holds states stored in globals

        ignStates|=((currSwitchStates.IGN_2&1)<<1); //shift IGN_2 over 1 and set it in ignStates
        ignStates|=(currSwitchStates.IGN_1&1);
        ignStored|=((globalSwitchStates->IGN_2&1)<<1);
        ignStored|=(globalSwitchStates->IGN_1&1);

        if (ignStored!=ignStates){ //new ignition state detected
            switch(ignStates){
                case 0x00:
                    CarState->ShouldArrayBeActivated = 0;
                    CarState->ShouldMotorBeActivated = 0;
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
                    CarState->ShouldArrayBeActivated = 1;
                    CarState->ShouldMotorBeActivated = 0;
                    if (arrayConThreadSpawned==false){ 
                        OSTaskCreate(
                            (OS_TCB*)&ArrayConnection_TCB,
                            (CPU_CHAR*)"Array Connection",
                            (OS_TASK_PTR)Task_ArrayConnection,
                            (void*)NULL,
                            (OS_PRIO)TASK_ARRAY_CONNECTION_PRIO,
                            (CPU_STK*)ArrayConnection_Stk,
                            (CPU_STK_SIZE)sizeof(ArrayConnection_Stk)/10,
                            (CPU_STK_SIZE)sizeof(ArrayConnection_Stk),
                            (OS_MSG_QTY)NULL,
                            (OS_TICK)NULL,
                            (void*)NULL,
                            (OS_OPT)(OS_OPT_TASK_STK_CLR|OS_OPT_TASK_STK_CHK),
                            (OS_ERR*)&err
                        );
                        arrayConThreadSpawned=true;
                    } else {
                        OSSemPost(
                            &ArrayConnectionChange_Sem4,
                            (OS_OPT) OS_OPT_POST_ALL,
                            (OS_ERR*) &err
                        );
                    }
                case 0x03:
                //Both are on
                    CarState->ShouldArrayBeActivated = 1;
                    CarState->ShouldMotorBeActivated = 1;
                    if(arrayConThreadSpawned==false){ 
                        OSTaskCreate(
                            (OS_TCB*)&ArrayConnection_TCB,
                            (CPU_CHAR*)"Array Connection",
                            (OS_TASK_PTR)Task_ArrayConnection,
                            (void*)NULL,
                            (OS_PRIO)TASK_ARRAY_CONNECTION_PRIO,
                            (CPU_STK*)ArrayConnection_Stk,
                            (CPU_STK_SIZE)sizeof(ArrayConnection_Stk)/10,
                            (CPU_STK_SIZE)sizeof(ArrayConnection_Stk),
                            (OS_MSG_QTY)NULL,
                            (OS_TICK)NULL,
                            (void*)NULL,
                            (OS_OPT)(OS_OPT_TASK_STK_CLR|OS_OPT_TASK_STK_CHK),
                            (OS_ERR*)&err
                        );
                        arrayConThreadSpawned=true;
                    } else {
                        OSSemPost(
                            &ArrayConnectionChange_Sem4,
                            (OS_OPT) OS_OPT_POST_ALL,
                            (OS_ERR*) &err
                        );
		            }
                    if(motorConThreadSpawned==false){
                        OSTaskCreate(
                            (OS_TCB*)&MotorConnection_TCB,
                            (CPU_CHAR*)"Motor Connection",
                            (OS_TASK_PTR)Task_MotorConnection,
                            (void*)NULL,
                            (OS_PRIO)TASK_MOTOR_CONNECTION_PRIO,
                            (CPU_STK*)ArrayConnection_Stk,
                            (CPU_STK_SIZE)sizeof(MotorConnection_Stk)/10,
                            (CPU_STK_SIZE)sizeof(MotorConnection_Stk),
                            (OS_MSG_QTY)NULL,
                            (OS_TICK)NULL,
                            (void*)NULL,
                            (OS_OPT)(OS_OPT_TASK_STK_CLR),
                            (OS_ERR*)&err
                        );
                        motorConThreadSpawned=true;
                    }else {
                        OSSemPost(
                            &MotorConnectionChange_Sem4,
                            (OS_OPT) OS_OPT_POST_ALL,
                            (OS_ERR*) &err
                        );
                    }
                case 0x02:
                    return -1; //Shouldn't happen - error case; only IGN2 is on
            }
       
        }
        
        //Signal LightsChange_Sem4 for UpdateLights Thread
        State curLightSwitches[]={
            currSwitchStates.lightSwitches.LEFT_SW,
            currSwitchStates.lightSwitches.RIGHT_SW,
            currSwitchStates.lightSwitches.HEADLIGHT_SW,
            currSwitchStates.lightSwitches.HZD_SW
        }; 

        State prevLightSwitches[]={
            globalSwitchStates->lightSwitches.LEFT_SW,
            globalSwitchStates->lightSwitches.RIGHT_SW,
            globalSwitchStates->lightSwitches.HEADLIGHT_SW,
            globalSwitchStates->lightSwitches.HZD_SW
        };       
        uint8_t lightSwitchStates = 0x00; //holds states read from Switches_Driver
        uint8_t lightSwitchStored = 0x00; //holds states stored in global
        for(uint8_t i = 0; i<4; i++){
            lightSwitchStates |= (curLightSwitches[i]<<i);
            lightSwitchStored |= (prevLightSwitches[i]<<i);
        }
        
        if(lightSwitchStored!=lightSwitchStates){
            //if any of the bits dont match, CarState needs to get updated and LightsChange must get signaled
            //TODO: use memcpy to move currSwitchStates.lightSwitches into globalSwitchStates->lightSwitches
            memcpy(&(globalSwitchStates->lightSwitches),&(currSwitchStates.lightSwitches),sizeof(currSwitchStates.lightSwitches));
            OSSemPost(
                &LightsChange_Sem4,
                (OS_OPT) OS_OPT_POST_ALL,
                (OS_ERR*)&err
            );
        }

        //DisplayChange_Sem4 + VelocityChangeSem4
        State readVelDispSwitches[] = {
            currSwitchStates.velDispSwitches.CRUZ_SW,
            currSwitchStates.velDispSwitches.CRUZ_EN,
            currSwitchStates.velDispSwitches.FWD_SW,
            currSwitchStates.velDispSwitches.REV_SW,
            currSwitchStates.velDispSwitches.REGEN_SW
        };
        State storeVelDispSwitches[] = {
            globalSwitchStates->velDispSwitches.CRUZ_SW,
            globalSwitchStates->velDispSwitches.CRUZ_EN,
            globalSwitchStates->velDispSwitches.FWD_SW,
            globalSwitchStates->velDispSwitches.REV_SW,
            globalSwitchStates->velDispSwitches.REGEN_SW
        };
        uint8_t velDispStates = 0x00;
        uint8_t velDispStored = 0x00;
        for(uint8_t i=0;i<3;i++){
            velDispStates |= ((readVelDispSwitches[i])<<i);
            velDispStored |= ((storeVelDispSwitches[i])<<i);
        }
        if(velDispStates != velDispStored){ //if any of the states don't match up both sem4 needs a signal
            //TODO: use memcpy to copy velDispSwitches of current to global
            memcpy(&(globalSwitchStates->velDispSwitches),&(currSwitchStates.velDispSwitches),sizeof(currSwitchStates.velDispSwitches));
            OSSemPost(
                &DisplayChange_Sem4,
                (OS_OPT) OS_OPT_POST_ALL,
                (OS_ERR*)&err
            );
            OSSemPost(
                &VelocityChange_Sem4,
                (OS_OPT) OS_OPT_POST_ALL,
                (OS_ERR*)&err
            );
        }
    
    //OS Time delay to allow for context switching. This locks us to reading the switches every 1 ms which should be fine.
    OSTimeDlyHMSM(0, 0, 0, 1, OS_OPT_TIME_HMSM_STRICT, &err);
    }
}
