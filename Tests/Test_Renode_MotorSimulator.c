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
static CANDATA_t newCD;
static uint8_t cycle_ctr = 0;
static uint16_t motor_force = 0;


// CURRENT CONTROLLED MODE
// Macros for calculating the velocity of the car
#define MS_TIME_DELAY_MILSEC 100
#define DECELERATION 2.0 // In m/s^2
#define CAR_MASS_KG 270
#define MAX_VELOCITY 20000.0f // rpm
#define MAX_CURRENT 50
#define WHEEL_RADIUS 0.2667f // In m
#define TORQUE_SLOPE 1.15384615384615f // in kgfcm/A


// VELOCITY CONTROL MODE

#define PROPORTION 1
#define INTEGRAL 0
#define DERIVATIVE 0
#define MAX_RPM 10741616 // 30m/s in RPM, decimal 2 places from right
#define DIVISOR 1

// Variables to help with PID calculation
static int32_t ErrorSum = 0;
static int32_t Error;
static int32_t Rate;
static int32_t PreviousError = 0;



static OS_TCB Task1TCB;
static CPU_STK Task1Stk[DEFAULT_STACK_SIZE];


//Function returns calculated current, calculated via PID
float Velocity_PID_Output() { 
    Error = setpointVelocity - velocity;
    ErrorSum = ErrorSum + Error;

    if (PreviousError == 0) {PreviousError = Error;} //init previous val first time

    Rate = (Error - PreviousError) / MS_TIME_DELAY_MILSEC;
    PreviousError = Error;     //updates previous err value

    if (((PROPORTION*(Error) + INTEGRAL*(ErrorSum) + DERIVATIVE*(Rate))/DIVISOR) > 1.0f) {
        return 1.0f;
    }
    else if (((PROPORTION*(Error) + INTEGRAL*(ErrorSum) + DERIVATIVE*(Rate))/DIVISOR) < 0.0f) {
        return 0.0f;
    }
    else {
        return (PROPORTION*(Error) + INTEGRAL*(ErrorSum) + DERIVATIVE*(Rate))/DIVISOR;
    }

    // TODO: Tweak the PID constants such that the output is a float from 0.0 to 1.0 
}

float CurrentToMotorForce(){ // Simulate giving the motor a current and returning a force
    
    return (current*(setpointCurrent*MAX_CURRENT)*TORQUE_SLOPE*100) / WHEEL_RADIUS;

}

float MotorForceToVelocity(){

    if (setpointVelocity > 0) {
        total_accel = ((float)motor_force / CAR_MASS_KG - DECELERATION);
    } else{
        total_accel = ((float)(-motor_force) / CAR_MASS_KG + DECELERATION);
    }

    return velocity + ((total_accel * MS_TIME_DELAY_MILSEC) / 1000); //Divide by 1000 to turn into seconds from ms;
}

void ReturnVelocity_CANDATA_t(){
    currCD.ID = MOTOR_DRIVE;
    currCD.idx = 0;

    memcpy(&currCD.data[0], &velocity, sizeof(velocity));
    memcpy(&currCD.data[4], &current, sizeof(current));

    CANbus_Send(currCD, CAN_BLOCKING, MOTORCAN);
}

    void Task1(void *arg)
{
    CANbus_Init(MOTORCAN);
    
    while (1)
    {
        OS_ERR err;

        ErrorStatus error = CANbus_Read(&newCD, CAN_NON_BLOCKING, MOTORCAN); // returns data value into newCD
        if (error == ERROR) // If there is no new value to read, use the old CAN data
        {
            newCD = oldCD;
        }

        // Parsing Measurements
        memcpy(&setpointVelocity, &newCD.data[0], sizeof setpointVelocity); // in RPM 
        memcpy(&setpointCurrent, &newCD.data[4], sizeof setpointCurrent); // Percent from 0.0 to 1.0
        
        if (abs(setpointVelocity) == MAX_VELOCITY){  // CURRENT CONTROLLED MODE

            current = 1.0f; // Calculated Current = 1

            // FORCE = Current(%) * Max Current (A) * Slope (kfgcm/A) * 100 / Wheel Radius (m)
            motor_force = CurrentToMotorForce();

            velocity = MotorForceToVelocity();

        } 
        else { // VELOCITY CONTROLLED MODE
            
            current = Velocity_PID_Output();

            motor_force = CurrentToMotorForce();

            velocity = MotorForceToVelocity();

        }


        ++cycle_ctr;
        if (cycle_ctr == 3)
        { // send currCD to read canbus every 300 ms
            cycle_ctr = 0;
            // Return velocity as part of currCD
            ReturnVelocity_CANDATA_t();
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

    CPU_Init();
    
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U)OSCfg_TickRate_Hz);

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
