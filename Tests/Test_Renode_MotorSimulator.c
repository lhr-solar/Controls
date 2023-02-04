#include "os.h"
#include "CANbus.h"
#include <stdio.h>
#include "Tasks.h"
#include <math.h>

// Global variables
static float setpointVelocity = 0.0f; 
static float velocity = 0.0f;
static float setpointCurrent = 0.0f;
static float current = 0.0f; 
static float total_accel = 0.0f;
static CANDATA_t oldCD;
static CANDATA_t currCD; // CANmsg we will be sending
static uint8_t cycle_ctr = 0;
static uint16_t motor_force = 0;


// CURRENT CONTROLLED MODE
// Macros for calculating the velocity of the car
#define MS_TIME_DELAY_MILSEC 100
#define DECELERATION 2.0 // In m/s^2
#define CAR_MASS_KG 270
#define MAX_VELOCITY 20000.0f // rpm
#define TORQUE_SLOPE 
#define MAX_CURRENT 50
#define WHEEL_RADIUS 0.2667f // In m
#define TORQUE_SLOPE 1.15384615384615f // in kgfcm/A


// VELOCITY CONTROL MODE

#define PROPORTION 1
#define INTEGRAL 0
#define DERIVATIVE 0
#define MAX_RPM 10741616 // 30m/s in RPM, decimal 2 places from right
#define DIVISOR 10000
// Variables to help with PID calculation
static int32_t ErrorSum = 0;
static int32_t Error;
static int32_t Rate;
static int32_t PreviousError = 0;


static OS_TCB Task1TCB;
static CPU_STK Task1Stk[DEFAULT_STACK_SIZE];


//Function returns new velocity, calculated via PID
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

void ReturnVelocity_CANDATA_t(){
    CANDATA_t message;
    message.ID = MOTOR_DRIVE;
    message.idx = 0;

    memcpy(&message.data[0], &velocity, sizeof(velocity));
    memcpy(&message.data[4], &current, sizeof(current));

    CANbus_Send(message, CAN_BLOCKING, MOTORCAN);
}

    void
    Task1(void *arg)
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
        memcpy(&setpointCurrent, &newCD.data[4], sizeof setpointCurrent); // Percent from 0.0 to 1.0
        
        if (abs(setpointVelocity) == MAX_VELOCITY){ 

            // CURRENT CONTROLLED MODE

            // FORCE = Current% * Max Current (A) * Slope (kfgcm/A) * 100 / Wheel Radius (m)
            motor_force = (current*MAX_CURRENT*TORQUE_SLOPE*100) / WHEEL_RADIUS;

            // Net acceleration is dependent on the force from the motor (based on current), mass of the car, 
            // and resistive forces which are being substituted with a constant 2m/s^2 negative acceleration
            if (setpointVelocity > 0) {
                total_accel = ((float)motor_force / CAR_MASS_KG - DECELERATION);
            } else{
                total_accel = ((float)(-motor_force) / CAR_MASS_KG + DECELERATION);
            }

            velocity += ((total_accel * MS_TIME_DELAY_MILSEC) / 1000); //Divide by 1000 to turn into seconds from ms

            // Return velocity as part of currCD
            ReturnVelocity_CANDATA_t();
        } 
        else {
            // VELOCITY CONTROLLED MODE
            velocity = Velocity_PID_Output();
            
            // Return velocity as part of currCD
            ReturnVelocity_CANDATA_t();
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
