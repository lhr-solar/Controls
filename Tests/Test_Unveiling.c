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
#include "os_cfg_app.h"
#include <bsp.h>
#include "daybreak_pins.h"
#include "BSP_GPIO.h"
#define BLINK_TO_FADE_OUT 3000u // 1s delay TODO: change this to be longer
#define FADE_OUT_TO_IN 1u       // 1min delay
#define FADE_IN_TO_BLINK 1u     // 1min delay
#define SET_BRIGHTNESS_DLY 200u // 200ms delay
#define BLINK_TWICE_DLY 250u    // 250ms delay


static OS_TCB Task1TCB;
static CPU_STK Task1Stk[DEFAULT_STACK_SIZE];

OS_TCB UnveilingLights_TCB;
static CPU_STK UnveilingLights_Stk[DEFAULT_STACK_SIZE];
static void Task_UnveilingLights(void *p_arg);

// Task_SetBrightness
//OS_TCB SetBrightness_TCB;
//static CPU_STK SetBrightness_Stk[DEFAULT_STACK_SIZE];
//static void Task_SetBrightness(int8_t *brightness);

// turn all lights on at maximum brightness
static void On_All(void)
{
    BSP_GPIO_Write_Pin(OS_FAULT_PORT, OS_FAULT, true);   // OS Fault LED as heartbeat
    BSP_GPIO_Write_Pin(TIMER_CLK_PORT, TIMER_CLK, true); // hazard button on dash MUST BE ON, otherwise won't work
    BSP_GPIO_Write_Pin(BRAKE_LIGHT_PORT, BRAKE_LIGHT, true);
}

// turn all lights off
static void Off_All(void)
{
    BSP_GPIO_Write_Pin(OS_FAULT_PORT, OS_FAULT, false);   // OS Fault LED as heartbeat
    BSP_GPIO_Write_Pin(TIMER_CLK_PORT, TIMER_CLK, false); // hazard button on dash MUST BE ON, otherwise won't work
    BSP_GPIO_Write_Pin(BRAKE_LIGHT_PORT, BRAKE_LIGHT, false);
}

static void Twice_All(void)
{
    OS_ERR err;

    Off_All();
    OSTimeDlyHMSM(0, 0, 0, BLINK_TWICE_DLY, OS_OPT_TIME_HMSM_STRICT, &err); // wait 250ms
    On_All();
    OSTimeDlyHMSM(0, 0, 0, BLINK_TWICE_DLY, OS_OPT_TIME_HMSM_STRICT, &err); // wait 250ms
    Off_All();
    OSTimeDlyHMSM(0, 0, 0, BLINK_TWICE_DLY, OS_OPT_TIME_HMSM_STRICT, &err); // wait 250ms
    On_All();
    OSTimeDlyHMSM(0, 0, 0, BLINK_TWICE_DLY, OS_OPT_TIME_HMSM_STRICT, &err); // wait 250ms
    Off_All();

    assertOSError(err);
}

// main lights task
static void Task_UnveilingLights(void *p_arg)
{
    OS_ERR err;

    while (1)
    {
        Twice_All();
        // OSTimeDlyHMSM(0, 0, 0, BLINK_TO_FADE_OUT, OS_OPT_TIME_HMSM_STRICT, &err);
        //(void)&Fade_All_Out;
        //OSTimeDlyHMSM(0, 0, 0, BLINK_TO_FADE_OUT, OS_OPT_TIME_HMSM_STRICT, &err);
        // OSTimeDlyHMSM(0, FADE_OUT_TO_IN, 0, 0, OS_OPT_TIME_HMSM_STRICT, &err);
        //(void)&Fade_All_In;
        //OSTimeDlyHMSM(0, FADE_IN_TO_BLINK, 0, 0, OS_OPT_TIME_HMSM_STRICT, &err);
        On_All();
        OSTimeDlyHMSM(0, 5u, 0, 0, OS_OPT_TIME_HMSM_STRICT, &err); //5 min
        Off_All();
        assertOSError(err);
    }
}

static void Lights_Task_Init()
{
    OS_ERR err;

    CPU_Init();
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U)OSCfg_TickRate_Hz);

    // init GPIO
    BSP_GPIO_Init(OS_FAULT_PORT, OS_FAULT, OUTPUT, false);
    BSP_GPIO_Init(TIMER_CLK_PORT, TIMER_CLK, OUTPUT, false);
    BSP_GPIO_Init(BRAKE_LIGHT_PORT, BRAKE_LIGHT, OUTPUT, false);
    BSP_GPIO_Init(BPS_FAULT_PORT, BPS_FAULT, OUTPUT, false);

    // UnveilingLights
    OSTaskCreate(
        (OS_TCB *)&UnveilingLights_TCB,
        (CPU_CHAR *)"UnveilingLights",
        (OS_TASK_PTR)Task_UnveilingLights,
        (void *)NULL,
        (OS_PRIO)TASK_PUT_IOSTATE_PRIO,
        (CPU_STK *)UnveilingLights_Stk,
        (CPU_STK_SIZE)WATERMARK_STACK_LIMIT,
        (CPU_STK_SIZE)TASK_SEND_CAR_CAN_STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)0,
        (void *)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR *)&err);
    assertOSError(err);
}

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

    for(int i=0; i<100;i+=1){
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
    
    testBoolComp(&UpdateDisplay_SetArray);
    testBoolComp(&UpdateDisplay_SetMotor);
    testPercentageCompSOC(&UpdateDisplay_SetSOC);
    testPercentageCompAccel(&UpdateDisplay_SetAccel);

    Display_Error();
    OSTimeDlyHMSM(0, 0, 3, 0, OS_OPT_TIME_HMSM_STRICT, &e);
    
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

        
    TaskSwHook_Init();
    Lights_Task_Init();
    assertOSError(err);

    OSStart(&err);
}