#include "common.h"
#include "config.h"
#include "os.h"
#include "Tasks.h"
#include "Display.h" 
// #include "bsp.h"
// #include "Contactors.h"
#include "UpdateDisplay.h"
#include "ReadCarCAN.h"
#include "ReadTritium.h"


static OS_TCB Task1TCB;
static CPU_STK Task1Stk[DEFAULT_STACK_SIZE];

void delay(){
    OS_ERR e;
    OSTimeDlyHMSM(0, 0, 0, 750, OS_OPT_TIME_HMSM_STRICT, &e);
}

void delay_short(){
    OS_ERR e;
    OSTimeDlyHMSM(0, 0, 0, 50, OS_OPT_TIME_HMSM_STRICT, &e);
}

void testBoolComp(UpdateDisplayError_t(*function)(bool)){
    function(false);
    delay();
    function(true);
    delay();
    function(false);
    delay();
}

void testPercentageCompAccel(UpdateDisplayError_t(*function)(uint8_t)){
    function(0);
    
    delay();
    function(25);
    
    delay();
    function(50);
    
    delay();
    function(75);
    
    delay();
    function(100);
    
    delay();
    function(0);
}

void testPercentageCompSOC(UpdateDisplayError_t(*function)(uint32_t)){

    for (int i = 0; i < 100; i++) {
        function(i);
        delay_short();
    }
    function(0);
}

void testTriStateComp(UpdateDisplayError_t(*function)(TriState_t)){
    function(STATE_0); // DISP_DISABLED & DISP_NEUTRAL
    
    delay();
    function(STATE_1); // DISP_ENABLED & DISP_FORWARD
    
    delay();
    function(STATE_2); // DISP_ACTIVE & DISP_REVERSE
    
    delay();
    function(STATE_0);
    
    delay();
}


void Task1(void *arg)
{   

    CPU_Init();
    Display_Init();
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U)OSCfg_TickRate_Hz);
    UpdateDisplay_Init();
    
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

    while(1){
        testTriStateComp(&UpdateDisplay_SetGear);
    
        testBoolComp(&UpdateDisplay_SetBrake);
        testBoolComp(&UpdateDisplay_SetBlink);

        testPercentageCompSOC(&UpdateDisplay_SetSOC);

        assertUpdateDisplayError(UPDATEDISPLAY_ERR_NONE); // SHould continue normally

        testPercentageCompAccel(&UpdateDisplay_SetAccel);

        Display_Error();
        OSTimeDlyHMSM(0, 0, 3, 0, OS_OPT_TIME_HMSM_STRICT, &e);

        assertUpdateDisplayError(UPDATEDISPLAY_ERR_DRIVER);
        
        Display_Reset();
        OSTimeDlyHMSM(0, 0, 3, 0, OS_OPT_TIME_HMSM_STRICT, &e);
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