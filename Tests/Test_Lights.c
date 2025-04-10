// Lighting show for LHRs unveiling 2025

#include "common.h"
#include "os_cfg_app.h"
#include "Tasks.h"
#include <bsp.h>
#include "daybreak_pins.h"
#include "BSP_GPIO.h"
#define BLINK_TO_FADE_OUT 1000u // 1s delay
#define FADE_OUT_TO_IN 1u       // 1min delay
#define FADE_IN_TO_BLINK 1u     // 1min delay

// Task_UnveilingLights
OS_TCB UnveilingLights_TCB;
static CPU_STK UnveilingLights_Stk[DEFAULT_STACK_SIZE];

// turn all lights on at maximum brightness
static void On_All(void)
{
    BSP_GPIO_Write_Pin(OS_FAULT_PORT, OS_FAULT, true);
    BSP_GPIO_Write_Pin(TIMER_CLK_PORT, TIMER_CLK, true); // hazard button on dash MUST BE ON, otherwise won't work
    BSP_GPIO_Write_Pin(BRAKE_LIGHT_PORT, BRAKE_LIGHT, true);
}

// turn all lights off
static void Off_All(void)
{
    BSP_GPIO_Write_Pin(OS_FAULT_PORT, OS_FAULT, false);
    BSP_GPIO_Write_Pin(TIMER_CLK_PORT, TIMER_CLK, false); // hazard button on dash MUST BE ON, otherwise won't work
    BSP_GPIO_Write_Pin(BRAKE_LIGHT_PORT, BRAKE_LIGHT, false);
}

// function to toggle/fade lights
static void Blink_All(void)
{
    // heartbeat using OS Fault LED on board
    BSP_GPIO_Toggle_Pin(OS_FAULT_PORT, OS_FAULT);

    // GPIO blink turn signals and brake light
    BSP_GPIO_Toggle_Pin(TIMER_CLK_PORT, TIMER_CLK); // hazard button on dash MUST BE ON, otherwise won't work
    BSP_GPIO_Toggle_Pin(BRAKE_LIGHT_PORT, BRAKE_LIGHT);
}

// fades all lights from maximum brightness to off
// TODO - make the timing configurable?
static void Fade_All_Out(void)
{
    On_All();

    // 90% pwm (high)
    // all between
    // 10% pwm (low)

    Off_All();
}

// fades all lights from off to maximum brightness
// TODO - make the timing configurable?
static void Fade_All_Out(void)
{
    Off_All();
}

// TODO: more functions for fade, blink individual light, rear lights, etc

// lights task
static void Task_UnveilingLights(void *p_arg)
{
    OS_ERR err;

    while (1)
    {
        Blink_All();
        OSTimeDlyHMSM(0, 0, 0, BLINK_TO_FADE_OUT, OS_OPT_TIME_HMSM_STRICT, &err);
        Fade_All_Out();
        OSTimeDlyHMSM(0, FADE_OUT_TO_IN, 0, 0, OS_OPT_TIME_HMSM_STRICT, &err);
        Fade_All_In();
        OSTimeDlyHMSM(0, FADE_IN_TO_BLINK, 0, 0, OS_OPT_TIME_HMSM_STRICT, &err);
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

int main()
{
    OS_ERR err;
    OSInit(&err);

    TaskSwHook_Init();
    Lights_Task_Init();

    OSStart(&err);
    assertOSError(err);
}

// BSP_GPIO_Init(TIMER_CLK_PORT, TIMER_CLK, OUTPUT, false);
// BSP_GPIO_Init(OS_FAULT_PORT, OS_FAULT, OUTPUT, false);

// while (1)
// {
//     volatile uint32_t waitTimeMain = 0;
//     while (waitTimeMain <= 99999)
//     {
//         volatile uint32_t waitTime = 0;
//         BSP_GPIO_Toggle_Pin(TIMER_CLK_PORT, TIMER_CLK);
//         BSP_GPIO_Toggle_Pin(OS_FAULT_PORT, OS_FAULT);
//         while (waitTime <= 12000)
//         {
//             waitTime++;
//         }
//         waitTimeMain++;
//     }

// waitTimeMain =
// while (waitTimeMain <= 99999)
// {
//     volatile uint32_t waitTime = 0;
//     BSP_GPIO_Toggle_Pin(TIMER_CLK_PORT, TIMER_CLK);
//     BSP_GPIO_Toggle_Pin(OS_FAULT_PORT, OS_FAULT);
//     while (waitTime <= 11000)
//     {
//         waitTime++;
//     }
//     waitTimeMain++;
// }
// BSP_GPIO_Toggle_Pin(TIMER_CLK_PORT, TIMER_CLK);
// BSP_GPIO_Toggle_Pin(OS_FAULT_PORT, OS_FAULT);
// waitTime = 0;
// while (waitTime <= 9999)
// {
//     waitTime++;
// }
//  }
