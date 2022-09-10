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
    Display_Init();

    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U)OSCfg_TickRate_Hz);

    OS_ERR err;

    while (1)
    {
        // Wait for startup screen to finish
        OSTimeDlyHMSM(0, 0, 3, 0, OS_OPT_TIME_HMSM_STRICT, &err);
        assertOSError(OS_MAIN_LOC, err);

        // Test info page
        Display_SetLeftBlink(true);
        Display_SendNext();
        // wait a few seconds to see if the blinking occurs
        OSTimeDlyHMSM(0, 0, 5, 0, OS_OPT_TIME_HMSM_STRICT, &err);
        assertOSError(OS_MAIN_LOC, err);
        Display_SetLeftBlink(false);
        Display_SendNext();

        Display_SetHeadLights(true);    // Does not exist
        Display_SendNext();
        Display_SetHeadLights(false);    // Does not exist
        Display_SendNext();

        // test 
        Display_SetRightBlink(true);
        Display_SendNext();
        // wait a few seconds to see if the blinking occurs
        OSTimeDlyHMSM(0, 0, 5, 0, OS_OPT_TIME_HMSM_STRICT, &err);
        assertOSError(OS_MAIN_LOC, err);
        Display_SetRightBlink(false);
        Display_SendNext();

        Display_SetCruiseEnable(true);
        Display_SendNext();
        Display_SetCruiseEnable(false);
        Display_SendNext();

        Display_SetRegenEnable(true);
        Display_SendNext();
        Display_SetRegenEnable(false);
        Display_SendNext();

        Display_SetVelocity(100); // DOes not exist
        Display_SendNext();
        
        Display_SetAccel(0); // deos not exist
        Display_SendNext();

        // does both the percentage integer and bar
        Display_SetSOC(0);
        Display_SendNext();
        Display_SetSOC(25);
        Display_SendNext();
        Display_SetSOC(50);
        Display_SendNext();
        Display_SetSOC(75);
        Display_SendNext();
        Display_SetSOC(100);
        Display_SendNext();

        Display_SetArray(true);
        Display_SendNext();
        Display_SetArray(false);
        Display_SendNext();

        Display_SetMotor(true);
        Display_SendNext();
        Display_SetMotor(false);
        Display_SendNext();

        // tests both BPV percentage and bar
        Display_SetBPV(0);
        Display_SendNext();
        Display_SetBPV(25);
        Display_SendNext();
        Display_SetBPV(50);
        Display_SendNext();
        Display_SetBPV(75);
        Display_SendNext();
        Display_SetBPV(100);
        Display_SendNext();

        // tests gears for displaying F,N, and R
        // The display 
        Display_SetGear(0);     // F
        Display_SendNext();
        Display_SetGear(1);     // N
        Display_SendNext();
        Display_SetGear(2);     // R
        Display_SendNext();
        
        // Test stuff on fault page
        FaultBitmap = 0x0420;
        OSErrLocBitmap = 0x6969;
        Display_Fault(OSErrLocBitmap, FaultBitmap);
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