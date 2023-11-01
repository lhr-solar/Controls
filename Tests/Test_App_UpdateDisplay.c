#include "common.h"
#include "config.h"
#include "os.h"
#include "Tasks.h"
#include "Display.h" 
// #include "bsp.h"
// #include "Contactors.h"
#include "UpdateDisplay.h"


static OS_TCB Task1TCB;
static CPU_STK Task1Stk[DEFAULT_STACK_SIZE];

void testBoolComp(UpdateDisplayError_t(*function)(bool)){
    OS_ERR e;

    function(false);
    OSTimeDlyHMSM(0, 0, 0, 200, OS_OPT_TIME_HMSM_STRICT, &e);
    function(true);
    OSTimeDlyHMSM(0, 0, 0, 200, OS_OPT_TIME_HMSM_STRICT, &e);
    function(false);
    OSTimeDlyHMSM(0, 0, 0, 200, OS_OPT_TIME_HMSM_STRICT, &e);
}

void testPercentageComp(UpdateDisplayError_t(*function)(uint8_t)){
    OS_ERR e;

    function(0);
    
    OSTimeDlyHMSM(0, 0, 0, 200, OS_OPT_TIME_HMSM_STRICT, &e);
    function(25);
    
    OSTimeDlyHMSM(0, 0, 0, 200, OS_OPT_TIME_HMSM_STRICT, &e);
    function(50);
    
    OSTimeDlyHMSM(0, 0, 0, 200, OS_OPT_TIME_HMSM_STRICT, &e);
    function(75);
    
    OSTimeDlyHMSM(0, 0, 0, 200, OS_OPT_TIME_HMSM_STRICT, &e);
    function(100);
    
    OSTimeDlyHMSM(0, 0, 0, 200, OS_OPT_TIME_HMSM_STRICT, &e);
    function(0);
    
    OSTimeDlyHMSM(0, 0, 0, 200, OS_OPT_TIME_HMSM_STRICT, &e);
}

void testTriStateComp(UpdateDisplayError_t(*function)(TriState_t)){
    OS_ERR e;

    function(STATE_0); // DISP_DISABLED & DISP_NEUTRAL
    
    OSTimeDlyHMSM(0, 0, 0, 200, OS_OPT_TIME_HMSM_STRICT, &e);
    function(STATE_1); // DISP_ENABLED & DISP_FORWARD
    
    OSTimeDlyHMSM(0, 0, 0, 200, OS_OPT_TIME_HMSM_STRICT, &e);
    function(STATE_2); // DISP_ACTIVE & DISP_REVERSE
    
    OSTimeDlyHMSM(0, 0, 0, 200, OS_OPT_TIME_HMSM_STRICT, &e);
    function(STATE_0);
    
    OSTimeDlyHMSM(0, 0, 0, 200, OS_OPT_TIME_HMSM_STRICT, &e);
}

void Task1(void *arg)
{   
    DisplayError_t error;

    CPU_Init();
    error = Display_Init();
    UpdateDisplay_Init();
    

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
    assertOSError(e);

    OSTimeDlyHMSM(0, 0, 7, 0, OS_OPT_TIME_HMSM_STRICT, &e);
    
    testTriStateComp(&UpdateDisplay_SetGear);
    
    UpdateDisplay_SetVelocity(12);
    
    OSTimeDlyHMSM(0, 0, 0, 200, OS_OPT_TIME_HMSM_STRICT, &e);
    UpdateDisplay_SetVelocity(345);
    
    OSTimeDlyHMSM(0, 0, 0, 200, OS_OPT_TIME_HMSM_STRICT, &e);
    UpdateDisplay_SetVelocity(6789);
    
    OSTimeDlyHMSM(0, 0, 0, 200, OS_OPT_TIME_HMSM_STRICT, &e);

    testTriStateComp(&UpdateDisplay_SetCruiseState);
    testTriStateComp(&UpdateDisplay_SetRegenState);
    testBoolComp(&UpdateDisplay_SetMotor);
    testBoolComp(&UpdateDisplay_SetArray);
    testPercentageComp(&UpdateDisplay_SetSOC);

    UpdateDisplay_SetSBPV(12);
    
    OSTimeDlyHMSM(0, 0, 0, 200, OS_OPT_TIME_HMSM_STRICT, &e);
    UpdateDisplay_SetSBPV(345);
    
    OSTimeDlyHMSM(0, 0, 0, 200, OS_OPT_TIME_HMSM_STRICT, &e);
    UpdateDisplay_SetSBPV(6789);
    
    OSTimeDlyHMSM(0, 0, 0, 200, OS_OPT_TIME_HMSM_STRICT, &e);

    testPercentageComp(&UpdateDisplay_SetAccel);

    Display_Error((error_code_t)error); // Testing Display_Error
    OSTimeDlyHMSM(0, 0, 3, 0, OS_OPT_TIME_HMSM_STRICT, &e);
    
    Display_Reset();
    OSTimeDlyHMSM(0, 0, 3, 0, OS_OPT_TIME_HMSM_STRICT, &e);
    
    while (1) {
        OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &e);
    }
};

int main()
{
    OS_ERR err;
    OSInit(&err);

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
    assertOSError(err);

    OSStart(&err);
}