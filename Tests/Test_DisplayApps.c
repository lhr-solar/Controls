#include "common.h"
#include "config.h"
#include "os.h"
#include "Tasks.h"
#include "Display.h"
// #include "bsp.h"
// #include "MotorController.h"
// #include "Contactors.h"
#include "UpdateDisplay.h"
#include "FaultState.h"

static OS_TCB Task1TCB;
static CPU_STK Task1Stk[DEFAULT_STACK_SIZE];

void testBoolComp(UpdateDisplay_Error_t(*function)(bool)){
    OS_ERR e;
    UpdateDisplay_Error_t err;

    err = function(false);
    OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &e);
    err = function(true);
    OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &e);
    err = function(false);
    OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &e);
}

void testPercentageComp(UpdateDisplay_Error_t(*function)(uint8_t)){
    OS_ERR e;
    UpdateDisplay_Error_t err;

    err = function(0);
    
    OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &e);
    err = function(25);
    
    OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &e);
    err = function(50);
    
    OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &e);
    err = function(75);
    
    OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &e);
    err = function(100);
    
    OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &e);
    err = function(0);
    
    OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &e);
}

void testTriStateComp(UpdateDisplay_Error_t(*function)(TriState_t)){
    OS_ERR e;
    UpdateDisplay_Error_t err;

    err = function(DISABLED);
    
    OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &e);
    err = function(ENABLED);
    
    OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &e);
    err = function(ACTIVE);
    
    OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &e);
    err = function(DISABLED);
    
    OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &e);
}

void Task1(void *arg)
{   
    Display_Error_t error;
    UpdateDisplay_Error_t err;

    CPU_Init();
    error = Display_Init();
    assertDisplayError(error);
    err = UpdateDisplay_Init();
    

    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U)OSCfg_TickRate_Hz);

    OS_ERR e;

    OSTaskCreate(
        (OS_TCB *)&UpdateDisplay_TCB,
        (CPU_CHAR *)"UpdateDisplay_TCB",
        (OS_TASK_PTR)Task_UpdateDisplay,
        (void *)NULL,
        (OS_PRIO)13,
        (CPU_STK *)UpdateDisplay_Stk,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE / 10,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)NULL,
        (void *)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR *)&e);
    assertOSError(OS_MAIN_LOC, e);

    OSTimeDlyHMSM(0, 0, 3, 0, OS_OPT_TIME_HMSM_STRICT, &e);
    
    testTriStateComp(&UpdateDisplay_SetGear);
    
    err = UpdateDisplay_SetVelocity(12);
    
    OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &e);
    err = UpdateDisplay_SetVelocity(345);
    
    OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &e);
    err = UpdateDisplay_SetVelocity(6789);
    
    OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &e);

    testTriStateComp(&UpdateDisplay_SetCruiseState);
    testTriStateComp(&UpdateDisplay_SetRegenState);
    testBoolComp(&UpdateDisplay_SetMotor);
    testBoolComp(&UpdateDisplay_SetArray);
    testPercentageComp(&UpdateDisplay_SetSOC);

    err = UpdateDisplay_SetSBPV(12);
    
    OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &e);
    err = UpdateDisplay_SetSBPV(345);
    
    OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &e);
    err = UpdateDisplay_SetSBPV(6789);
    
    OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &e);

    testPercentageComp(&UpdateDisplay_SetAccel);

    error = Display_Fault(OSErrLocBitmap, FaultBitmap);
    assertDisplayError(error);
    OSTimeDlyHMSM(0, 0, 3, 0, OS_OPT_TIME_HMSM_STRICT, &e);
    
    error = Display_Reset();
    assertDisplayError(error);
    OSTimeDlyHMSM(0, 0, 3, 0, OS_OPT_TIME_HMSM_STRICT, &e);
    
    while (1) {
        OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &e);
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
        (OS_PRIO)12,
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