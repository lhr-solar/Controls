// General Imports.
#include "common.h"
#include "config.h"
#include "os.h"
#include "Tasks.h"

// Device specfic imports.
#include "UpdateDisplay.h"
#include "ReadTritium.h"

#define STACK_SIZE 128

static OS_TCB Task1TCB;
static OS_TCB Task2TCB;
static CPU_STK Task1Stk[STACK_SIZE];
static CPU_STK Task2Stk[STACK_SIZE];

int main() {
    OS_ERR err;
    OSInit(&err);
    if (err != OS_ERR_NONE) {
        printf("OS error code %d\n", err);
    }

    // SendTritium
    OSTaskCreate(
        (OS_TCB*)&SendTritium_TCB,
        (CPU_CHAR*)"SendTritium",
        (OS_TASK_PTR)Task_SendTritium,
        (void*) NULL,
        (OS_PRIO)TASK_SEND_TRITIUM_PRIO,
        (CPU_STK*)SendTritium_Stk,
        (CPU_STK_SIZE)WATERMARK_STACK_LIMIT/10,
        (CPU_STK_SIZE)TASK_SEND_TRITIUM_STACK_SIZE,
        (OS_MSG_QTY) 0,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    );

    if (err != OS_ERR_NONE) {
        printf("SendTritium Task error code %d\n", err);
    }

    // ReadTritium
    OSTaskCreate(
        (OS_TCB*)&ReadTritium_TCB,
        (CPU_CHAR*)"ReadTritium",
        (OS_TASK_PTR)Task_ReadTritium,
        (void*) NULL,
        (OS_PRIO)TASK_READ_TRITIUM_PRIO,
        (CPU_STK*)SendTritium_Stk,
        (CPU_STK_SIZE)WATERMARK_STACK_LIMIT/10,
        (CPU_STK_SIZE)TASK_READ_TRITIUM_STACK_SIZE,
        (OS_MSG_QTY) 0,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    );

    if (err != OS_ERR_NONE) {
        printf("ReadTritium Task error code %d\n", err);
    }

    // UpdateDisplay
    OSTaskCreate(
        (OS_TCB *)&UpdateDisplay_TCB,
        (CPU_CHAR *)"UpdateDisplay_TCB",
        (OS_TASK_PTR)Task_UpdateDisplay,
        (void *)NULL,
        (OS_PRIO)TASK_UPDATE_DISPLAY_PRIO,
        (CPU_STK *)UpdateDisplay_Stk,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE / 10,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)NULL,
        (void *)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR *)&err);

    if (err != OS_ERR_NONE) {
        printf("UpdateDisplay Task error code %d\n", err);
    }

    OSStart(&err);

    if (err != OS_ERR_NONE) {
        printf("OS error code %d\n", err);
    }

    return 0;
}