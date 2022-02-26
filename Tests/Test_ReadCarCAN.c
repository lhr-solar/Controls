#include "common.h"
#include "config.h"
#include "os.h"
#include "Tasks.h"
#include "CarState.h"
#include "CANbus.h"
#include "stm32f4xx.h"
#include "Contactors.h"

#define THRESHOLD 3
void Task1(void *);

void main(void)
{
    static OS_TCB Task1_TCB;
    static CPU_STK Task1_STK[128];

    OS_ERR err;
    OSInit(&err);

    // Initialized with error
    if (err != OS_ERR_NONE)
    {
        printf("OS error code %d\n", err);
    }

    OSTaskCreate(
        (OS_TCB *)&Task1_TCB,
        (CPU_CHAR *)"Task 1",
        (OS_TASK_PTR)Task1,
        (void *)NULL,
        (OS_PRIO)3,
        (CPU_STK *)Task1_STK,
        (CPU_STK_SIZE)128 / 10,
        (CPU_STK_SIZE)128,
        (OS_MSG_QTY)NULL,
        (OS_TICK)NULL,
        (void *)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR *)&err);

    // Task not created
    if (err != OS_ERR_NONE)
    {
        printf("Task error code %d\n", err);
    }

    OSStart(&err);
    if (err != OS_ERR_NONE)
    {
        printf("OS error code %d\n", err);
    }
}

void Task1(void *p_arg)
{

    OS_ERR err;
    CPU_TS ts;

    car_state_t car;

    /*CANPayload_t payload1;
    payload1.bytes = 8;
    payload1.data.w = 1;

    CANPayload_t payload2;
    payload2.bytes = 8;
    payload2.data.w = 0;*/

    CPU_Init();
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U)OSCfg_TickRate_Hz);

    CANbus_Init();
    Contactors_Init(ARRAY);

    // car_state_t *car = (car_state_t *)p_arg;

    uint8_t buffer[8]; // buffer for CAN message

    static int faultCounter = 0;

    uint32_t canId;

    while (1)
    {
        // Check if BPS sent us a message
        if (CANbus_Read(&canId, buffer, CAN_BLOCKING) == SUCCESS)
        {
            // If charge_enable, set regen flag
            if (canId == CHARGE_ENABLE)
            {
                if (buffer[0] == 0)
                {
                    car.IsRegenBrakingAllowed = OFF;
                }
                else
                {
                    car.IsRegenBrakingAllowed = ON;
                }
                // car->IsRegenBrakingAllowed = (buffer[0] == 0) ? OFF : ON;
                Contactors_Set(ARRAY, car.IsRegenBrakingAllowed);
                faultCounter = 0;
            }
            else
            {
                // If we didn't get a message, something might have gone wrong
                faultCounter++;
            }
        }

        if (faultCounter >= THRESHOLD)
        {
            car.IsRegenBrakingAllowed = OFF;
            Contactors_Set(ARRAY, OFF);
        }

        OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_NON_STRICT, &err);

        if (err != OS_ERR_NONE)
        {
            car.ErrorCode.ReadCANErr = ON;
        }
    }

    /*printf("Spawned ReadCarCAN\n");

    // Simulate BPS enabling and disabling charge
    OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &err);
    printf("Is regen braking allowed? %s\n", car.IsRegenBrakingAllowed ? "yes" : "no");
    CANbus_Send(CHARGE_ENABLE, payload2); // disable charging
    OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_NON_STRICT, &err);
    printf("Is regen braking allowed? %s\n", car.IsRegenBrakingAllowed ? "yes" : "no");
    CANbus_Send(CHARGE_ENABLE, payload1); // enable charging
    OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_NON_STRICT, &err);
    printf("Is regen braking allowed? %s\n", car.IsRegenBrakingAllowed ? "yes" : "no");

    // Make sure the task can't fault when suspended
    OSTaskSuspend(&ReadCarCAN_TCB, &err);
    OSTimeDlyHMSM(0, 0, 3, 0, OS_OPT_TIME_HMSM_NON_STRICT, &err); // If task was active, this would trigger a fault
    OSTaskResume(&ReadCarCAN_TCB, &err);
    printf("Is regen braking allowed? %s\n", car.IsRegenBrakingAllowed ? "yes" : "no"); // should be yes
    OSTimeDlyHMSM(0, 0, 3, 0, OS_OPT_TIME_HMSM_NON_STRICT, &err); // should trigger a fault now
    printf("Is regen braking allowed? %s\n", car.IsRegenBrakingAllowed ? "yes" : "no"); // should be no
    */
    OSTaskDel(NULL, &err);
}