#include "os.h"
#include "CANbus.h"
#include <stdio.h>
#include "Tasks.h"

//global variables
static float velocity; 
static float total_accel;
static CANDATA_t oldCD;
//lookup table goes here
static int ctr = 0;
#define MS_TIME_DELAY_MS 100
#define TIME_DELAY_S (MS_TIME_DELAY_MS / 1000)
#define DECELERATION 2.0
#define CAR_MASS_KG 270

static OS_TCB Task1TCB;
static CPU_STK Task1Stk[DEFAULT_STACK_SIZE];

void Task1(void *arg)
{
    while (1)
    {
        OS_ERR err;
        CANDATA_t newCD;
        ErrorStatus error = CANbus_Read(&newCD, CAN_NON_BLOCKING, MOTORCAN); // returns data value into newCD
        if (error == ERROR)
        {
            newCD = oldCD;
        }
        total_accel = ((/*force from LUT   /   */  CAR_MASS_KG) - DECELERATION);
        velocity += (total_accel * TIME_DELAY_S);
        ++ctr;
        if (ctr == 3)
        { // send message to read canbus every 300 ms
            ctr = 0;
            CANbus_Send(newCD, CAN_NON_BLOCKING, MOTORCAN);
        }
        oldCD = newCD;
        OSTimeDlyHMSM(0, 0, 0, MS_TIME_DELAY_MS, OS_OPT_TIME_HMSM_STRICT, &err);
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
