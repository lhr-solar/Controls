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
    switch_states_t *storedSwitchStates = &(((car_state_t*)p_arg)->SwitchStates); //pointer to global switchstates
    switch_states_t readSwitchStates; //holds states read from driver reads

    // load readSwitchStates
    readSwitchStates = (switch_states_t){
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
    bool arrayConThreadSpawned = false;
    bool motorConThreadSpawned = false;
    while(1){
        //Spawns arrayConnection thread and MotorConnection thread when it is appropriate
        uint8_t ignStates = 0x00; //holds states read from Switches Driver
        uint8_t ignStored = 0x00; //holds states stored in globals
        ignStates = ignStates|((readSwitchStates.IGN_2&1)<<1); //shift IGN_2 over 1 and set it in ignStates
        ignStates = ignStates|(readSwitchStates.IGN_1&1);
        ignStored = ignStored|((storedSwitchStates->IGN_2&1)<<1);
        ignStored = ignStates|(storedSwitchStates->IGN_1&1);
        if (ignStored!=ignStates){
            switch(ignStates){
                case 0x00:
                    //neither is on
                    storedSwitchStates->IGN_1 = 0;
                    storedSwitchStates->IGN_2 = 0;
                    //TODO: add car turn off code?
                    ((car_state_t*)p_arg)->ShouldArrayBeActivated = 0;
                    ((car_state_t*)p_arg)->ShouldMotorBeActivated = 0;
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
                    storedSwitchStates->IGN_1=1;
                    storedSwitchStates->IGN_2=0;
                    #pragma region
                    if (arrayConThreadSpawned=false){ 
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
                    }
                    #pragma endregion
                case 0x03:
                //Both are on
                    storedSwitchStates->IGN_2=1;
                    storedSwitchStates->IGN_1=1;
                    //OSTASKCREATE activate array, OSTASKCREATE activate motor
                    #pragma region
                    if(arrayConThreadSpawned=false){ 
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
                    }
                    if(motorConThreadSpawned){
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
                    }
                    #pragma endregion
                case 0x02:
                    return -1; //Shouldn't happen - error case; only IGN2 is on
            }
       
        }
        
        //Signal LightsChange_Sem4 for UpdateLights Thread
        State readlightSwitches[]={
            readSwitchStates.LEFT_SW,
            readSwitchStates.RIGHT_SW,
            readSwitchStates.HEADLIGHT_SW,
            readSwitchStates.HZD_SW
        }; 
        State storeLightSwitches[]={
            storedSwitchStates->LEFT_SW,
            storedSwitchStates->RIGHT_SW,
            storedSwitchStates->HEADLIGHT_SW,
            storedSwitchStates->HZD_SW
        };       
        uint8_t lightSwitchStates = 0x00; //holds states read from Switches_Driver
        uint8_t lightSwitchStored = 0x00; //holds states stored in global
        for(uint8_t i = 0; i<3; i++){
            lightSwitchStates = lightSwitchStates | (readlightSwitches[i]<<i);
            lightSwitchStored = lightSwitchStored | (storeLightSwitches[i]<<i);
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
            readSwitchStates.CRUZ_SW,
            readSwitchStates.CRUZ_EN,
            readSwitchStates.FWD_SW,
            readSwitchStates.REV_SW,
            readSwitchStates.REGEN_SW
        };
        State storeVelSwitches[] = {
            storedSwitchStates->CRUZ_SW,
            storedSwitchStates->CRUZ_EN,
            storedSwitchStates->FWD_SW,
            storedSwitchStates->REV_SW,
            storedSwitchStates->REGEN_SW
        };
        uint8_t velStates = 0x00; //holds read states of switches in least sig 5 bits
        uint8_t velStored = 0x00; //hold stored global states of velocity switches in least sig bits
        for(uint8_t i = 0; i<5;i++){
            velStates = velStates | ((readVelSwitches[i]&1)<<i);
            velStored = velStored | ((storeVelSwitches[i]&1)<<i);
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
            readSwitchStates.CRUZ_EN,
            readSwitchStates.CRUZ_SW,
            readSwitchStates.REGEN_SW
        };
        State storeDispSwitches[] = {
            storedSwitchStates->CRUZ_EN,
            storedSwitchStates->CRUZ_SW,
            storedSwitchStates->REGEN_SW
        };
        uint8_t dispStates = 0x00;
        uint8_t dispStored = 0x00;
        for(uint8_t i=0;i<3;i++){
            dispStates = dispStates | ((readDispSwitches[i])<<i);
            dispStored = dispStored | ((storeDispSwitches[i])<<i);
        }
        if(dispStates != dispStored){ //if any of the states don't match up sem4 needs a signal
            OSSemPost(
                &DisplayChange_Sem4,
                (OS_OPT) OS_OPT_POST_ALL,
                (OS_ERR*)&err
            );
        }


        //set global state to match what was read
        *storedSwitchStates = readSwitchStates; 

    }
}