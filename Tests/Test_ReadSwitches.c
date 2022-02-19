#include "common.h"
#include "config.h"
#include "os.h"
#include "Tasks.h"
#include "CarState.h"
#include "CANbus.h"

void Task1(void *);

void main(void){
    static OS_TCB Task1_TCB;
    static CPU_STK Task1_STK[128];
    OS_ERR err;
    OSInit(&err);

    //init error
    if(err != OS_ERR_NONE){
        printf("OS error code %d\n", err);
    }

    OSTaskCreate(
        (OS_TCB*)&Task1_TCB,
        (CPU_CHAR*)"Task 1",
        (OS_TASK_PTR)Task1,
        (void*)NULL,
        (OS_PRIO)3,
        (CPU_STK*)Task1_STK,
        (CPU_STK_SIZE)128/10,
        (CPU_STK_SIZE)128,
        (OS_MSG_QTY)NULL,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    );
    if (err != OS_ERR_NONE) {
        printf("Task error code %d\n", err);
    }

    OSStart(&err);
    if (err != OS_ERR_NONE) {
        printf("OS error code %d\n", err);
    }

}

void Task1 (void *p_arg){
    OS_ERR err;
    CPU_TS ts;

    car_state_t car = (car_state_t){
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        0,
        0,
        0,
        (switch_states_t) {
            (State)OFF,
            (State)OFF,
            (velocity_switches_t){
                (State)OFF,
                (State)OFF,
                (State)OFF,
                (State)OFF,
                (State)OFF
            },
            (light_switches_t){
                (State)OFF,
                (State)OFF,
                (State)OFF,
                (State)OFF
            }
        },
        (CruiseRegenSet)ACCEL,
        (State)OFF,
        0,
        (State)OFF,
        (RegenMode)REGEN_OFF,
        (State)OFF,
        (State)OFF,
        (error_code_t){
            (State) OFF,
            (State) OFF,
            (State) OFF,
            (State) OFF,
            (State) OFF,
            (State) OFF,
            (State) OFF,
            (State) OFF,
            (State) OFF
        },
        (motor_error_code_t) {
            OFF,
            OFF,
            OFF,
            OFF,
        }
    };
    CPU_Init();
    OS_CPU_SysTickInit();

    OSTaskCreate(
        (OS_TCB*)&ReadSwitches_TCB,
        (CPU_CHAR*)"ReadSwitches",
        (OS_TASK_PTR)Task_ReadSwitches,
        (void*)&car,
        (OS_PRIO)4,
        (CPU_STK*)ReadSwitches_Stk,
        (CPU_STK_SIZE)128/10,
        (CPU_STK_SIZE)128,
        (OS_MSG_QTY)NULL,
        (OS_TICK)NULL,
        (void*)&car,
        (OS_OPT)(OS_OPT_TASK_STK_CLR|OS_OPT_TASK_STK_CHK),
        (OS_ERR*)&err
    );

    

}
