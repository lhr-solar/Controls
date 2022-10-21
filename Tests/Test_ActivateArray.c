#include "ArrayConnection.h"
#include "Tasks.h"
#include "CANbus.h"


static OS_TCB Task1TCB;
static CPU_STK Task1Stk[128];
static car_state_t car_state;

void Task1(void *p_arg) {
    
    car_state_t *car = (car_state_t *) p_arg;

    CPU_Init();
    OS_CPU_SysTickInit();

    OS_ERR err;
    CPU_TS ts;

    CANPayload_t payload1;
    payload1.bytes = 8;
    payload1.data.d = 1;

    CANPayload_t payload2;
    payload2.bytes = 8;
    payload2.data.d = 0;

    OSTaskCreate(
        (OS_TCB*)&ArrayConnection_TCB,
        (CPU_CHAR*)"ActivateArray",
        (OS_TASK_PTR)Task_ArrayConnection,
        (void*)car,
        (OS_PRIO)TASK_ARRAY_CONNECTION_PRIO,
        (CPU_STK*)ArrayConnection_Stk,
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
    
    // delay a bit before starting
    OSTimeDlyHMSM(0, 0, 5, 0, OS_OPT_TIME_HMSM_NON_STRICT, &err);
    
    // Simulate BPS enabling and disabling charge
    OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &err);
    printf("Is regen braking allowed? %s\n", car->IsRegenBrakingAllowed == true ? "yes" : "no"); // should be yes
    CANbus_Send(CHARGE_ENABLE, payload2); // disable charging
    OSTimeDlyHMSM(0, 0, 1, 300, OS_OPT_TIME_HMSM_NON_STRICT, &err);
    printf("Is regen braking allowed? %s\n", car->IsRegenBrakingAllowed == true ? "yes" : "no"); // should be no
    CANbus_Send(CHARGE_ENABLE, payload1); // enable charging
    OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_HMSM_NON_STRICT, &err);
    printf("Is regen braking allowed? %s\n\n\n", car->IsRegenBrakingAllowed == true ? "yes" : "no"); // should be yes

    // Make sure the task can't fault when suspended
    car->ShouldArrayBeActivated = false;
    OSSemPost(&ArrayConnectionChange_Sem4, OS_OPT_POST_ALL, &err); // deactivate array
    OSTimeDlyHMSM(0, 0, 3, 0, OS_OPT_TIME_HMSM_NON_STRICT, &err); // If task was active, this would trigger a fault
    printf("Contactor state: %s\n", Contactors_Get(ARRAY) == true ? "on" : "off");
    car->ShouldArrayBeActivated = true;
    OSSemPost(&ArrayConnectionChange_Sem4, OS_OPT_POST_ALL, &err); // re-activate array
    printf("Is regen braking allowed? %s\n", car->IsRegenBrakingAllowed == true ? "yes" : "no"); // should be yes
    OSTimeDlyHMSM(0, 0, 6, 0, OS_OPT_TIME_HMSM_NON_STRICT, &err); // should trigger a fault now
    printf("Is regen braking allowed? %s\n", car->IsRegenBrakingAllowed == true ? "yes" : "no"); // should be no
    
    while (1) {
        OSTimeDlyHMSM(0, 0, 2, 0, OS_OPT_TIME_HMSM_NON_STRICT, &err);
    }
    
    OSTaskDel(NULL, &err);
}

/**
 * Waits 5 seconds, then signals for the array to activate
 * Prints seconds to the screen
 * @param void
 * @return 
 */
int main(void) {
    OS_ERR err;
    car_state.IsRegenBrakingAllowed = true;
    OSInit(&err);

    if (err != OS_ERR_NONE) {
            printf("OS error code %d\n", err);
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

    if (err != OS_ERR_NONE) {
        printf("Task error code %d\n", err);
    }

    OSSemCreate(&ArrayConnectionChange_Sem4, "ArrayConnectionChange", 0, &err);

    

    printf("Starting\n");

    OSStart(&err);
    if (err != OS_ERR_NONE) {
        printf("OS error code %d\n", err);
    }
    
}