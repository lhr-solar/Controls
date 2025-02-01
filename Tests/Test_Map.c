/**
 * Test file for testing floating point register restoration on context switch
 * 
 * To be tested on hardware, if it doesn't hard fault then it worked!
 */


#include "common.h"
#include "config.h"
#include "stm32f4xx.h"
#include <bsp.h>
#include "SendTritium.h"

static OS_TCB Task1_TCB;
static CPU_STK Task1_Stk[128];


void Task1(void *p_arg) {
    CPU_Init();
    // OS_CPU_SysTickInit();
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U) OSCfg_TickRate_Hz);
    OS_ERR err;

    // These test cases are based on the actual uses of mapToPercent in SendTritium.c
    printf("\n\r============ Testing mapToPercent ============\n\r");
    printf("\n\rInput (0-100) Output (0-100)\n\r");
    printf("Input Value: %d, Output Value: %f, Expected Value: %f\n", 0, mapToPercent(0, 0, 100, 0, 100), 0.0);
    printf("Input Value: %d, Output Value: %f, Expected Value: %f\n", 15, mapToPercent(15, 0, 100, 0, 100), 0.15);
    printf("Input Value: %d, Output Value: %f, Expected Value: %f\n", 74, mapToPercent(74, 0, 100, 0, 100), 0.74);
    printf("Input Value: %d, Output Value: %f, Expected Value: %f\n", 100, mapToPercent(100, 0, 100, 0, 100), 1.0);
    printf("Input Value: %d, Output Value: %f, Expected Value: %f\n", 115, mapToPercent(115, 0, 100, 0, 100), 1.0);

    printf("\n\rInput (15-100) Output (0-100)\n\r");
    printf("Input Value: %d, Output Value: %f, Expected Value: %f\n", 0, mapToPercent(0, 15, 100, 0, 100), 0.0);
    printf("Input Value: %d, Output Value: %f, Expected Value: %f\n", 15, mapToPercent(15, 15, 100, 0, 100), 0.0);
    printf("Input Value: %d, Output Value: %f, Expected Value: %f\n", 16, mapToPercent(16, 15, 100, 0, 100), 0.01);
    printf("Input Value: %d, Output Value: %f, Expected Value: %f\n", 74, mapToPercent(74, 15, 100, 0, 100), 0.69);
    printf("Input Value: %d, Output Value: %f, Expected Value: %f\n", 100, mapToPercent(100, 15, 100, 0, 100), 1.0);
    printf("Input Value: %d, Output Value: %f, Expected Value: %f\n", 115, mapToPercent(115, 15, 100, 0, 100), 1.0);

    printf("\n\rInput (35-100) Output (0-100)\n\r");
    printf("Input Value: %d, Output Value: %f, Expected Value: %f\n", 0, mapToPercent(0, 35, 100, 0, 100), 0.0);
    printf("Input Value: %d, Output Value: %f, Expected Value: %f\n", 35, mapToPercent(35, 35, 100, 0, 100), 0.0);
    printf("Input Value: %d, Output Value: %f, Expected Value: %f\n", 48, mapToPercent(48, 35, 100, 0, 100), 0.2);
    printf("Input Value: %d, Output Value: %f, Expected Value: %f\n", 76, mapToPercent(76, 35, 100, 0, 100), 0.63);
    printf("Input Value: %d, Output Value: %f, Expected Value: %f\n", 100, mapToPercent(100, 35, 100, 0, 100), 1.0);
    printf("Input Value: %d, Output Value: %f, Expected Value: %f\n", 115, mapToPercent(115, 35, 100, 0, 100), 1.0);
}

int main(void) {
    OS_ERR err;
    OSInit(&err);
    if(err != OS_ERR_NONE){
        printf("OS error code %d\n",err);
    }
    OSTaskCreate(
        (OS_TCB*)&Task1_TCB,
        (CPU_CHAR*)"Task1",
        (OS_TASK_PTR)Task1,
        (void*)NULL,
        (OS_PRIO)4,
        (CPU_STK*)Task1_Stk,
        (CPU_STK_SIZE)sizeof(Task1_Stk)/10,
        (CPU_STK_SIZE)sizeof(Task1_Stk),
        (OS_MSG_QTY)0,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_SAVE_FP|OS_OPT_TASK_STK_CHK),
        (OS_ERR*)&err
    );

    if (err != OS_ERR_NONE) {
        printf("Task1 error code %d\n", err);
    }
    OSStart(&err);
    if (err != OS_ERR_NONE) {
        printf("OS error code %d\n", err);
    }

    return 0;
}