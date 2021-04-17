#include "common.h"
#include "config.h"
#include "UpdateVelocity.h"

#define INITIAL_SWITCH_STATES {OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF}
#define INITIAL_BLINKER_STATES {OFF, OFF, OFF}

car_state_t carState = {0.0f, 0.0f, 0.0f, 0.0f, 0, 0, 0, INITIAL_SWITCH_STATES, INITIAL_BLINKER_STATES, OFF, OFF};

OS_TCB UpdateRandomPedal_TCB;
CPU_STK UpdateRandomPedal_Stk[DEFAULT_STACK_SIZE];

void Task_UpdateRandomPedal(void* p_arg){
    car_state_t *car_state = (car_state_t *) p_arg;

    OS_ERR err;

    car_state->CruiseControlVelocity = 20;
    while(1){    
        car_state->AccelPedalPercent = rand() % 100;
        
        car_state->CruiseControlEnable = rand() % 2;
        car_state->CruiseControlSet = rand() % 2;

        printf("Accel: %d | CC Enable: %d | CC Set: %d\n", car_state->AccelPedalPercent, car_state->CruiseControlEnable, car_state->CruiseControlSet);
        printf("desired velocity: %f, desired motor current: %f\n", car_state->DesiredVelocity, car_state->DesiredMotorCurrent);
        // Delay of few milliseconds (10)
        OSTimeDlyHMSM (0, 0, 0, 10, OS_OPT_TIME_HMSM_STRICT, &err);
    }
}

int main() {
    OS_ERR err;
    OSInit(&err);
    
    printf("inside main");

    if(err != OS_ERR_NONE){
        printf("OS error code %d\n", err);
    }

    OSTaskCreate(
        (OS_TCB*)&UpdateVelocity_TCB,
        (CPU_CHAR*)"UpdateVelocity",
        (OS_TASK_PTR)Task_UpdateVelocity,
        (void*) &carState,
        (OS_PRIO)TASK_UPDATE_VELOCITY_PRIO,
        (CPU_STK*)UpdateVelocity_Stk,
        (CPU_STK_SIZE)WATERMARK_STACK_LIMIT/10,
        (CPU_STK_SIZE)TASK_UPDATE_VELOCITY_STACK_SIZE,
        (OS_MSG_QTY)NULL,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    );

    if (err != OS_ERR_NONE) {
        printf("Task error code %d\n", err);
    }

    OSTaskCreate(
        (OS_TCB*)&UpdateRandomPedal_TCB,
        (CPU_CHAR*)"Random Pedal Task",
        (OS_TASK_PTR)Task_UpdateRandomPedal,
        (void*)&carState,
        (OS_PRIO)2,
        (CPU_STK*)UpdateRandomPedal_Stk,
        (CPU_STK_SIZE)128/10,
        (CPU_STK_SIZE)128,
        (OS_MSG_QTY)NULL,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    );


    OS_CPU_SysTickInit();


    OSStart(&err);
    if (err != OS_ERR_NONE) {
        printf("OS error code %d\n", err);
    }
}