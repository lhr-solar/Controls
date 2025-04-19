#include "common.h"
#include "config.h"
#include "os.h"
#include "Tasks.h"
#include "Display.h" 
// #include "Contactors.h"
#include "UpdateDisplay.h"
#include "ReadCarCAN.h"
#include "ReadTritium.h"
#include "os_cfg_app.h"
#include "daybreak_pins.h"
#include "BSP_GPIO.h"
#include "BSP_PWM.h"
#include "bsp.h"
#include "StatusLeds.h"

#define BLINK_TO_FADE_OUT 3000u // 1s delay TODO: change this to be longer
#define FADE_OUT_TO_IN 1u       // 1min delay
#define FADE_IN_TO_BLINK 1u     // 1min delay
#define SET_BRIGHTNESS_DLY 200u // 200ms delay
#define BLINK_TWICE_DLY 250u    // 250ms delay

#define LIGHTS_DEF 1
#define DISPLAY_DEF 1

#define PRIO_DEF 6

void delay_short();
void delay();

#if DISPLAY_DEF
static OS_TCB Task1TCB;
static CPU_STK Task1Stk[DEFAULT_STACK_SIZE];
#endif 



#if LIGHTS_DEF
static OS_TCB UnveilingLights_TCB;
static CPU_STK UnveilingLights_Stk[DEFAULT_STACK_SIZE];
static void Task_UnveilingLights(void *p_arg);


// main lights task
static void Task_UnveilingLights(void *p_arg)
{
    int duty = 2;
    int add = 1;
    while (1) {
        BSP_PWM_Set_Duty_Cycle(duty);
        duty += add;

        if(duty == 30) add = -1;
        else if(duty == 1) add = 1;

        delay_short();
    }
}

static void Lights_Task_Init() {
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U)OSCfg_TickRate_Hz);
    OS_ERR err;
    // init GPIO
    BSP_GPIO_Init(OS_FAULT_PORT, OS_FAULT, OUTPUT, false);
    //BSP_GPIO_Init(TIMER_CLK_PORT, TIMER_CLK, OUTPUT, false);
    //BSP_GPIO_Init(BRAKE_LIGHT_PORT, BRAKE_LIGHT, OUTPUT, false);
    BSP_GPIO_Init(BPS_FAULT_PORT, BPS_FAULT, OUTPUT, false);

    BSP_PWM_Init(69420, 1, true);

    // Status_Leds_Init();
    // Status_Leds_Toggle(CONTROLS_FAULT_LED);

    BSP_PWM_Set_State(INDICATOR_PWM, true);
    BSP_PWM_Set_State(BRAKE_PWM, true);


    // UnveilingLights
    OSTaskCreate(
        (OS_TCB *)&UnveilingLights_TCB,
        (CPU_CHAR *)"UnveilingLights",
        (OS_TASK_PTR)Task_UnveilingLights,
        (void *)NULL,
        (OS_PRIO)PRIO_DEF,
        (CPU_STK *)UnveilingLights_Stk,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE / 10,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)0,
        (void *)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR *)&err);
    assertOSError(err);
}
#endif

void delay(){
    OS_ERR e;
    OSTimeDlyHMSM(0, 0, 0, 750, OS_OPT_TIME_HMSM_STRICT, &e);
    assertOSError(e);
}

void delay_short(){
    OS_ERR e;
    OSTimeDlyHMSM(0, 0, 0, 50, OS_OPT_TIME_HMSM_STRICT, &e);
    assertOSError(e);
}

#if DISPLAY_DEF
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
    OS_ERR e;
    // CPU_Init();
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U)OSCfg_TickRate_Hz);
    Display_Init();
    UpdateDisplay_Init();

    // OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U)OSCfg_TickRate_Hz);

    

    OSTaskCreate(
        (OS_TCB *)&UpdateDisplay_TCB,
        (CPU_CHAR *)"UpdateDisplay_TCB",
        (OS_TASK_PTR)Task_UpdateDisplay,
        (void *)NULL,
        (OS_PRIO) PRIO_DEF,
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

        //Display_Error();
        //OSTimeDlyHMSM(0, 0, 3, 0, OS_OPT_TIME_HMSM_STRICT, &e);
        
        //Display_Reset();
        //OSTimeDlyHMSM(0, 0, 3, 0, OS_OPT_TIME_HMSM_STRICT, &e);
        
    }
};
#endif

int main()
{
    OS_ERR err;
    CPU_Init();
    OSInit(&err);
    assertOSError(err);
    // CPU_Init();

    #if DISPLAY_DEF
    // create tester thread
    OSTaskCreate(
        (OS_TCB *)&Task1TCB,
        (CPU_CHAR *)"Task 1",
        (OS_TASK_PTR)Task1,
        (void *)NULL,
        (OS_PRIO)PRIO_DEF,
        (CPU_STK *)Task1Stk,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE / 10,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)NULL,
        (void *)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR *)&err);
    #endif
    
    TaskSwHook_Init();
    #if LIGHTS_DEF
    Lights_Task_Init();
    #endif
    assertOSError(err);

    OSStart(&err);

    delay();
    delay_short();
    //On_All();
}