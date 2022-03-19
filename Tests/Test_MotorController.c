/**
 * File for testing Motor Controller and verifying CAN3 functionality
 * 
 * 
 * This file spins the motor up to .5amps for 200ms, then 1 amp for 200ms, 
 * and then non-regen brakes for 200ms and repeats. 
 */ 
#include "common.h"
#include "config.h"
#include "MotorController.h"
#include "os.h"
#include "Tasks.h"

static OS_TCB Task1TCB;
static CPU_STK Task1Stk[128];
static car_state_t car_state;


#define BUSCURRENT 10

void Task1(void* arg){
    CPU_Init();
    OS_CPU_SysTickInit();

    OS_ERR err;
    CPU_TS ts;


    MotorController_Init(1.0f);
    float unatttainable_Velocity;
    float desiredCurrent; 
    float currentSetPoint;



    while(1){
        unatttainable_Velocity = 2500.0f;
        desiredCurrent = 0.5f; //spin up to .5 amps worth
        currentSetPoint = desiredCurrent/BUSCURRENT;
        MotorController_Drive(unatttainable_Velocity,currentSetPoint);
        OSTimeDlyHMSM(0,0,0,200,OS_OPT_TIME_HMSM_STRICT, &err);
        if(err != OS_ERR_NONE){
            while(1){
                volatile int x = 0;
            }
        }
        desiredCurrent = 1.0f; //1 amps worth
        currentSetPoint = desiredCurrent/BUSCURRENT;
        MotorController_Drive(unatttainable_Velocity,currentSetPoint);
        OSTimeDlyHMSM(0,0,0,200,OS_OPT_TIME_HMSM_STRICT, &err);
        if(err != OS_ERR_NONE){
            while(1){
                volatile int x = 0;
            }
        }
        
        
        MotorController_Drive(unatttainable_Velocity,0); //non-regen brake
        MotorController_Drive(0,0);
        OSTimeDlyHMWSM(0,0,0,200,OS_OPT_TIME_HMSM_STRICT, &err);
        if(err != OS_ERR_NONE){
            while(1){
                volatile int x = 0;
            }
        }
    }
    
}

int main(){    
    OS_ERR err;
    OSInit(&err);
    if(err != OS_ERR_NONE){
        while(1){
            volatile int x = 0;
        }
    }
    OSSemCreate( //driver depends on fault state semaphore/thread
        &FaultState_Sem4,
        "Fault State Semaphore",
        0,
        &err
    );
    if(err != OS_ERR_NONE){
        while(1){
            volatile int x = 0;
        }
    }

    OSTaskCreate(
        (OS_TCB*)&FaultState_TCB,
        (CPU_CHAR*)"Fault State",
        (OS_TASK_PTR)&Task_FaultState,
        (void*)NULL,
        (OS_PRIO)1,
        (CPU_STK*)FaultState_Stk,
        (CPU_STK_SIZE)128/10,
        (CPU_STK_SIZE)128,
        (OS_MSG_QTY)NULL,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    );
    if(err != OS_ERR_NONE){
        while(1){
            volatile int x = 0;
        }
    }



    OSTaskCreate(
        (OS_TCB*)&Task1TCB,
        (CPU_CHAR*)"Task 1",
        (OS_TASK_PTR)Task1,
        (void*)&car_state,
        (OS_PRIO)13,
        (CPU_STK*)Task1Stk,
        (CPU_STK_SIZE)128/10,
        (CPU_STK_SIZE)128,
        (OS_MSG_QTY)NULL,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    );
    if(err != OS_ERR_NONE){
        while(1){
            volatile int x = 0;
        }
    }
    OSStart(&err);
    
    if(err != OS_ERR_NONE){
        while(1){
            volatile int x = 0;
        }
    }

    while(1){volatile int x = 0;}
    
    
}