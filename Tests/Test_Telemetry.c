#include "os.h"
#include "Tasks.h"
#include <bsp.h>
#include "config.h"
#include "common.h"
#include "Pedals.h"
#include "Minions.h"
#include "Contactors.h"
#include "CANbus.h"
#include "CAN_Queue.h"

static OS_TCB Task1TCB;
static CPU_STK Task1Stk[DEFAULT_STACK_SIZE];

void Task1(void *arg)
{   
    CPU_Init();
    
    CANbus_Init(CARCAN, NULL, 0);
    CAN_Queue_Init();
    // BSP_UART_Init(UART_2);
    Pedals_Init();
    Minion_Init();
    Contactors_Init();

    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U)OSCfg_TickRate_Hz);

    Minion_Error_t Merr;
    OS_ERR err;

    // Initialize Telemetry
    OSTaskCreate(
        (OS_TCB*)&Telemetry_TCB,
        (CPU_CHAR*)"Telemetry",
        (OS_TASK_PTR)Task_Telemetry,
        (void*)NULL,
        (OS_PRIO)TASK_TELEMETRY_PRIO,
        (CPU_STK*)Telemetry_Stk,
        (CPU_STK_SIZE)WATERMARK_STACK_LIMIT,
        (CPU_STK_SIZE)TASK_TELEMETRY_STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)0,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    );
    assertOSError(OS_MAIN_LOC, err);

    bool lightState = false;
    State contactorState = OFF;

    CANDATA_t msg;
    msg.ID = MOTOR_STATUS;
    msg.idx = 0;
    msg.data[0] = 0x12;
    msg.data[1] = 0x34;
    msg.data[2] = 0x56;
    msg.data[3] = 0x78;
    msg.data[4] = 0x9A;
    msg.data[5] = 0xBC;
    msg.data[6] = 0xDE;
    msg.data[7] = 0xF0;
    
    while (1){
        // Switches can be modified through hardware

        // Check light
        lightState = !lightState;
        Minion_Write_Output(BRAKELIGHT, lightState, &Merr);

        // Check contactors (HAVE NOTHING HOOKED UP TO CONTACTORS)
        contactorState = contactorState == OFF ? ON : OFF;
        Contactors_Set(MOTOR_CONTACTOR, contactorState, true);
        Contactors_Set(ARRAY_CONTACTOR, contactorState, true);

        (msg.ID)++;
        if(msg.ID > 0x24F){
            msg.ID = MOTOR_STATUS;
        }
        CAN_Queue_Post(msg);

        CANbus_Read(&msg, CAN_BLOCKING, CARCAN);

        OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &err);
        // Use a logic analyzer to read the CAN line and see if the data shows up correctly    
    }
}

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
        (OS_PRIO)5,
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