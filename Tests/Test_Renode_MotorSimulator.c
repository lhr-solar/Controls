#include "os.h"
#include "CANbus.h"
#include <stdio.h>
#include "Tasks.h"

// Global variables
static float velocity; 
static float current; 
static float total_accel;
static CANDATA_t oldCD;
static int ctr = 0;
static float motor_force;
static int forceLUTIndex;

// FORCE LUT 
// FORCELUT Size
#define FORCELUT_SIZE 21 
// Force Lookup Table: Indexed by Current%toIndex, returns a force applied by current % (in Newtons)
const int FORCELUT[FORCELUT_SIZE] = {
    0,
    235,
    471,
    706,
    941,
    1176,
    1412,
    1647,
    1882,
    2118,
    2353,
    2588,
    2824,
    3059,
    3294,
    3529,
    3765,
    4000,
    4235,
    4471,
    4706
};

// Macros for calculation the velocity of the car
#define MS_TIME_DELAY_MS 100
#define DECELERATION 2.0
#define CAR_MASS_KG 270

static OS_TCB Task1TCB;
static CPU_STK Task1Stk[DEFAULT_STACK_SIZE];

// Helper function to convert from current to desired FORCELUT index
inline int currentPercentToIndex(float currentPercent){
        return ((int)(currentPercent*FORCELUT_SIZE));
    }

// Helper function to convert time delay from ms to s
inline float millisToS(int timeInMS) {
    return (((float)MS_TIME_DELAY_MS) / 1000);
}    

void Task1(void *arg)
{
    while (1)
    {
        OS_ERR err;
        CANDATA_t newCD;

        ErrorStatus error = CANbus_Read(&newCD, CAN_NON_BLOCKING, MOTORCAN); // returns data value into newCD
        if (error == ERROR) // If there is no new value to read, use the old CAN data
        {
            newCD = oldCD;
        }

        // Parsing Measurements
        velocity = newCD.data[0]; // m/s
        current = newCD.data[4]; // Percent from 0.0 to 1.0
        
        
        forceLUTIndex = currentPercentToIndex(current); // Converts current to index for FORCELUT
        motor_force = FORCELUT[forceLUTIndex];

        // Net acceleration is dependent on the force from the motor (based on current), mass of the car, 
        // and resistive forces which are being substituted with a constant 2m/s^2 negative acceleration
        total_accel = ((motor_force / CAR_MASS_KG) - DECELERATION);
        velocity += (total_accel * mmillisToS(MS_TIME_DELAY_MS));

        ++ctr;
        if (ctr == 3)
        { // send message to read canbus every 300 ms
            ctr = 0;
            CANbus_Send(newCD, CAN_NON_BLOCKING, MOTORCAN);
        }
        oldCD = newCD; // Update old CAN data to match most recent values received
        OSTimeDlyHMSM(0, 0, 0, MS_TIME_DELAY_MS, OS_OPT_TIME_HMSM_STRICT, &err);
        assertOSError(OS_MAIN_LOC, err);
    }
}
    
int main()
{
    // Disable interrupts
    __disable_irq();

    OS_ERR err;
    OSInit(&err);
    assertOSError(OS_MAIN_LOC, err);

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

    // Enable interrupts
    __enable_irq();

    //start_OS
    OSStart(&err);
    assertOSError(OS_MAIN_LOC, err);

    while (1);

    return 0;

    }
