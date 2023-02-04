#include "os.h"
#include "CANbus.h"
#include <stdio.h>
#include "Tasks.h"
#include <math.h>

// Global variables
static float setpointVelocity = 0.0f; 
static float velocity = 0.0f;
static float current = 0.0f; 
static float total_accel = 0.0f;
static CANDATA_t oldCD;
static CANDATA_t currCD; // CANmsg we will be sending
static uint8_t cycle_ctr = 0;
static uint16_t motor_force = 0;
static uint8_t forceLUTIndex = 0;

// FORCE LUT 
// FORCELUT Size
#define FORCELUT_SIZE 21 
// Force Lookup Table: Indexed by Current%toIndex, returns a force applied by current % (in Newtons)
static const int FORCELUT[FORCELUT_SIZE] = {
    0,
    309,
    618,
    926,
    1235,
    1544,
    1853,
    2162,
    2471,
    2779,
    3088,
    3397,
    3706,
    4015,
    4324,
    4632,
    4941,
    5250,
    5559,
    5868,
    6176
}; //TODO: Waiting for confirmation on newly calculated force values

// CURRENT CONTROLLED MODE
// Macros for calculating the velocity of the car
#define MS_TIME_DELAY_MILSEC 100
#define DECELERATION 2.0 // In m/s^2
#define CAR_MASS_KG 270
#define MAX_VELOCITY 20000.0f // rpm


// VELOCITY CONTROL MODE
//TODO: tune PID with actual pack and fans, and then change values below to appropiate value
#define PROPORTION 1
#define INTEGRAL 1
#define DERIVATIVE 1
#define MAX_RPM 179026 // 30m/s in RPM, decimal 2 places from right
#define DIVISOR 10000
// Variables to help with PID calculation
static int32_t ErrorSum = 0;
static int32_t Error;
static int32_t Rate;
static int32_t PreviousError = 0;


static OS_TCB Task1TCB;
static CPU_STK Task1Stk[DEFAULT_STACK_SIZE];

// Helper function to convert from current to desired FORCELUT index
inline int currentPercentToIndex(float currentPercent){
    return ((int)(currentPercent*(FORCELUT_SIZE - 1)));
}    

//
float Velocity_PID_Output() {
    Error = setpointVelocity - velocity;
    ErrorSum = ErrorSum + Error;

    if (PreviousError == 0) {PreviousError = Error;} //init previous val first time

    Rate = (Error - PreviousError) / MS_TIME_DELAY_MILSEC;
    PreviousError = Error;     //updates previous err value

    if (((PROPORTION*(Error) + INTEGRAL*(ErrorSum) + DERIVATIVE*(Rate))/DIVISOR) > MAX_RPM) {
        return MAX_RPM;
    }
    else if (((PROPORTION*(Error) + INTEGRAL*(ErrorSum) + DERIVATIVE*(Rate))/DIVISOR) < -MAX_RPM) {
        return -MAX_RPM;
    }
    else {
        return (PROPORTION*(Error) + INTEGRAL*(ErrorSum) + DERIVATIVE*(Rate))/DIVISOR;
    }

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
        memcpy(&setpointVelocity, &newCD.data[0], sizeof setpointVelocity); // in RPM 
        memcpy(&current, &newCD.data[4], sizeof current); // Percent from 0.0 to 1.0
        
        if (abs(setpointVelocity) == MAX_VELOCITY){ 

            // CURRENT CONTROLLED MODE

            forceLUTIndex = currentPercentToIndex(current); // Converts current to index for FORCELUT
            motor_force = FORCELUT[forceLUTIndex];

            // Net acceleration is dependent on the force from the motor (based on current), mass of the car, 
            // and resistive forces which are being substituted with a constant 2m/s^2 negative acceleration
            if (setpointVelocity < 0) {
                total_accel = ((float)motor_force / CAR_MASS_KG - DECELERATION);
            } else{
                total_accel = ((float)(-motor_force) / CAR_MASS_KG + DECELERATION);
            }

            velocity += ((total_accel * MS_TIME_DELAY_MILSEC) / 1000); //Divide by 1000 to turn into seconds from ms

            // TODO: Return velocity as part of currCD

        } else {
            
            // VELOCITY CONTROLLED MODE

            velocity = Velocity_PID_Output(setpointVelocity);
            

            // TODO: Return velocity as part of currCD
             
        }


        ++cycle_ctr;
        if (cycle_ctr == 3)
        { // send message to read canbus every 300 ms
            cycle_ctr = 0;
            CANbus_Send(newCD, CAN_NON_BLOCKING, MOTORCAN);
        }
        oldCD = newCD; // Update old CAN data to match most recent values received
        OSTimeDlyHMSM(0, 0, 0, MS_TIME_DELAY_MILSEC, OS_OPT_TIME_HMSM_STRICT, &err);
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
