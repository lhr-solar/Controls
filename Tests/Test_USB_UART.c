// test for USB-UART functionality on Daybreak 2025 controls leader
// sends hello world every 250ms - read using putty serial on usb

#include "common.h"
#include "os_cfg_app.h"
#include "Tasks.h"
#include <bsp.h>
#include "daybreak_pins.h"

#define DLY_MS 250u

// Task_UARTTest
OS_TCB Task1_TCB;
static CPU_STK Task1_Stk[DEFAULT_STACK_SIZE];
OS_TCB UARTTest_TCB;
CPU_STK UARTTest_Stk[DEFAULT_STACK_SIZE];

static void testUART(void) {
    // heartbeat
    BSP_GPIO_Write_Pin(BPS_FAULT_PORT, BPS_FAULT, !BSP_GPIO_Get_State(BPS_FAULT_PORT, BPS_FAULT));

    // uart write test
    char test_uart_msg[] = "hello world!";
    BSP_UART_Write(USB, test_uart_msg, 13);
}

static void Task_UARTTest(void *p_arg) {
    OS_ERR err;

    while (1) {
        testUART();
        OSTimeDlyHMSM(0, 0, 0, DLY_MS, OS_OPT_TIME_HMSM_STRICT, &err);
        assertOSError(err);
    }  
}

void Task1(void *p_arg) {
    OS_ERR err;

    CPU_Init();
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U) OSCfg_TickRate_Hz);
    BSP_UART_Init(USB);
    BSP_GPIO_Init(BPS_FAULT_PORT, BPS_FAULT, OUTPUT, false);
    
    // UARTTest
    OSTaskCreate(
        (OS_TCB *)&UARTTest_TCB,
        (CPU_CHAR *)"UARTTest",
        (OS_TASK_PTR)Task_UARTTest,
        (void *)NULL,
        (OS_PRIO)TASK_PUT_IOSTATE_PRIO,
        (CPU_STK *)UARTTest_Stk,
        (CPU_STK_SIZE)WATERMARK_STACK_LIMIT,
        (CPU_STK_SIZE)TASK_SEND_CAR_CAN_STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)0,
        (void *)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR *)&err);
    assertOSError(err);
}

int main(){
    OS_ERR err;
    OSInit(&err);

    TaskSwHook_Init();

    OSTaskCreate(
        (OS_TCB*)&Task1_TCB,
        (CPU_CHAR*)"Task1",
        (OS_TASK_PTR)Task1,
        (void*)NULL,
        (OS_PRIO)4,
        (CPU_STK*)Task1_Stk,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE/10,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR|OS_OPT_TASK_STK_CHK),
        (OS_ERR*)&err
    );
    assertOSError(err);

    OSStart(&err);
    assertOSError(err);
}