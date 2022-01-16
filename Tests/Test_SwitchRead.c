#include "common.h"
#include "config.h"
#include <os.h>
#include "Tasks.h"

/**
 * This file is a basic test file that spawns the readSwitches thread, 
 * and another thread that prints those states to the console. 
 * 
*/
static car_state_t CarState;
static switch_states_t SwitchStates = {
    OFF,
    OFF,
    (velocity_switches_t){
        OFF,
        OFF,
        OFF,
        OFF,
        OFF
    },
    (light_switches_t){
        OFF,
        OFF,
        OFF,
        OFF
    }
};
static OS_ERR err;


void main(void){
    OSInit(&err);
    OSTaskCreate(
        (OS_TCB*)&ReadSwitches_TCB,
        (CPU_CHAR*)"Read Switches Task",
        (OS_TASK_PTR)Task_ReadSwitches,
        (void*)&CarState,
        (OS_PRIO)TASK_ARRAY_CONNECTION_PRIO,
        (CPU_STK*)ReadSwitches_Stk,
        (CPU_STK_SIZE)sizeof(ReadSwitches_Stk)/10,
        (CPU_STK_SIZE)sizeof(ReadSwitches_Stk),
        (OS_MSG_QTY)NULL,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR|OS_OPT_TASK_STK_CHK),
        (OS_ERR*)&err
    );

    // Task not created
    if (err != OS_ERR_NONE) {
        printf("Task error code %d\n", err);
    }

    OSStart(&err);
    if (err != OS_ERR_NONE) {
        printf("OS error code %d\n", err);
    }
}

/**
 * Basic Printer function
*/
void printerThread(void){
    while(1){
        switch_states_t SwitchStates = CarState.SwitchStates;
        velocity_switches_t velDispSwitches = SwitchStates.velDispSwitches;
        light_switches_t lightSwitches = SwitchStates.lightSwitches;
        printf(
            "IGN_1: %i,\
             IGN_2: %i, \
             LEFT_SW: %i, \
             RIGHT_SW: %i, \
             HEADLIGHT_SW: %i, \
             HZD_SW: %i, \
             CRUZ_SW: %i, \
             CRUZ_EN: %i, \
             FWD_SW: %i, \
             REV_SW: %i, \
             REGEN_SW: %i \
             \r",
             SwitchStates.IGN_1,
             SwitchStates.IGN_2,
             lightSwitches.LEFT_SW,
             lightSwitches.RIGHT_SW,
             lightSwitches.HEADLIGHT_SW,
             lightSwitches.HZD_SW,
             velDispSwitches.CRUZ_SW,
             velDispSwitches.CRUZ_EN,
             velDispSwitches.FWD_SW,
             velDispSwitches.REV_SW,
             velDispSwitches.REGEN_SW
        );
        OSTimeDlyHMSM(0, 0, 0, 2, OS_OPT_TIME_HMSM_STRICT, &err); //only run every 2 ms
    }
}
