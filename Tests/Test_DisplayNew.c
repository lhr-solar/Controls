#include "common.h"
#include "config.h"
#include "os.h"
#include "Tasks.h"
#include "Display.h"
#include "bsp.h"
#include "MotorController.h"
#include "Contactors.h"

static OS_TCB Task1TCB;
static CPU_STK Task1Stk[DEFAULT_STACK_SIZE];

void Task1(void *arg)
{
    CPU_Init();
    Contactors_Init();
    Display_Init();

    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U)OSCfg_TickRate_Hz);
    SupplementalVoltage = 200;
    StateOfCharge = 12345678;
    RegenEnable = true;
    Contactors_Enable(ARRAY_CONTACTOR);
    Contactors_Set(ARRAY_CONTACTOR, true);

    OS_ERR err;

    OSTaskCreate(
        (OS_TCB *)&SendDisplay_TCB,
        (CPU_CHAR *)"SendDisplay_TCB",
        (OS_TASK_PTR)Task_SendDisplay,
        (void *)NULL,
        (OS_PRIO)13,
        (CPU_STK *)SendDisplay_Stk,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE / 10,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)NULL,
        (void *)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR *)&err);
    assertOSError(OS_MAIN_LOC, err);

    while (1)
    {
        OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_HMSM_STRICT, &err);
        assertOSError(OS_MAIN_LOC, err);

        SupplementalVoltage += 100;
        StateOfCharge += 100000;
        if (RegenEnable == true)
        {
            RegenEnable = false;
        }
        else
        {
            RegenEnable = true;
        }

        // Testing driver directly
        /*Display_SetSBPV(SupplementalVoltage);
        Display_SetChargeState(StateOfCharge);
        Display_SetRegenEnabled(RegenEnable);
        Display_SetVelocity(25.0);*/
    }
};

int main()
{
    OS_ERR err;
    OSInit(&err);
    OSSemCreate( // create fault sem4
        &FaultState_Sem4,
        "Fault State Semaphore",
        0,
        &err);
    assertOSError(OS_MAIN_LOC, err);

    OSTaskCreate( // create fault task
        (OS_TCB *)&FaultState_TCB,
        (CPU_CHAR *)"Fault State",
        (OS_TASK_PTR)&Task_FaultState,
        (void *)NULL,
        (OS_PRIO)1,
        (CPU_STK *)FaultState_Stk,
        (CPU_STK_SIZE)128 / 10,
        (CPU_STK_SIZE)128,
        (OS_MSG_QTY)0,
        (OS_TICK)NULL,
        (void *)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR *)&err);
    assertOSError(OS_MAIN_LOC, err);

    // create tester thread
    OSTaskCreate(
        (OS_TCB *)&Task1TCB,
        (CPU_CHAR *)"Task 1",
        (OS_TASK_PTR)Task1,
        (void *)NULL,
        (OS_PRIO)13,
        (CPU_STK *)Task1Stk,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE / 10,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)NULL,
        (void *)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR *)&err);
    assertOSError(OS_MAIN_LOC, err);

    OSStart(&err);
}