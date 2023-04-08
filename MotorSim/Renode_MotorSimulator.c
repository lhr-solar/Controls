#include "os.h"
#include "CANbus.h"
#include <stdio.h>
#include "Tasks.h"
#include <math.h>
#include "BSP_UART.h"

// Global variables
static float velocityRPM = 0.0f;
static float velocityMPS = 0.0f;

static CANDATA_t prevMsg;
static uint8_t cycle_ctr = 0;


// MOTOR FORCE TO VELOCITY MACROS
#define MS_TIME_DELAY_MILSEC 10 
#define DRAG_COEFFICIENT 0.09f
#define FRONTAL_SURFACE_AREA 0.61f//in m^2
#define AIR_DENSITY 1.202f // in kg/m^3
#define CAR_MASS_KG 320
#define MAX_VELOCITY 20000.0f // rpm
#define MAX_CURRENT 50 // in Amps
#define WHEEL_RADIUS 0.2667f // In m
#define TORQUE_SLOPE 75.0f // in kgfcm/A


// VELOCITY CONTROL MODE MACROS
#define PROPORTION 50
#define INTEGRAL   0
#define DERIVATIVE 0
#define MAX_RPM 10741616 // 30m/s in RPM, decimal 2 places from right
#define DIVISOR 1

// Variables to help with PID calculation
static float ErrorSum = 0;
static float Error;
static float Rate;
static float PreviousError = 0;


static OS_TCB Task1TCB;
static CPU_STK Task1Stk[DEFAULT_STACK_SIZE];


//function to convert from RPM to M/S
float RevPerMinToMetersPerSec(float velocityRPM){
    //converting the velocity from RPM to M/S
    return ((velocityRPM * (2 * M_PI * WHEEL_RADIUS)) / 60); 
}

//Function to convert from M/S to RPM
float MetersPerSecToRevPerMin(float velocityMPS){
    //converting the velocity from M/S to RPM
    return ((velocityMPS * 60) / (2 * M_PI * WHEEL_RADIUS)); 
}

//Function returns calculated current, calculated via PID
float Velocity_PID_Output(float setpointVelocity) { 
    Error = RevPerMinToMetersPerSec(setpointVelocity) - velocityMPS;
    
    ErrorSum = ErrorSum + Error;

    if (PreviousError == 0) {PreviousError = Error;} //init previous val first time

    Rate = (Error - PreviousError) / MS_TIME_DELAY_MILSEC;
    PreviousError = Error;     //updates previous err value

    if (((PROPORTION*(Error) + INTEGRAL*(ErrorSum) + DERIVATIVE*(Rate))/DIVISOR) > 1.0f) {
        return 1.0f;
    }
    else if (((PROPORTION*(Error) + INTEGRAL*(ErrorSum) + DERIVATIVE*(Rate))/DIVISOR) < -1.0f) {
        return -1.0f;
    }
    else {
        return (PROPORTION*(Error) + INTEGRAL*(ErrorSum) + DERIVATIVE*(Rate))/DIVISOR;
    }

    // TODO: Tweak the PID constants such that the output is a float from -1.0 to 1.0 
}

float CurrentToMotorForce(float current, float setpointCurrent){ // Simulate giving the motor a current and returning a force
    
    // FORCE = Current(%) * (CurrentSetpoint * Max Current (A)) * Slope (kfgcm/A) * 9.8 / 100 / Wheel Radius (m)
    return (current * (setpointCurrent * MAX_CURRENT) * TORQUE_SLOPE * 9.8 / 100) / WHEEL_RADIUS;

}

float MotorForceToVelocity(float motorForce){ 
    //drag calculations
    float totalAccel = 0.0f;

    float predictedVel = velocityMPS + (((float)motorForce / CAR_MASS_KG)*(MS_TIME_DELAY_MILSEC) / (1000)); //in m/s

    float deceleration = (0.5 * DRAG_COEFFICIENT * FRONTAL_SURFACE_AREA * AIR_DENSITY * (velocityMPS*velocityMPS));
    
    if (predictedVel <= 0){
        totalAccel = ((float)(motorForce + deceleration) / CAR_MASS_KG); //in m/s^2
    } else if (predictedVel > 0){
        totalAccel = ((float)(motorForce - deceleration) / CAR_MASS_KG); //in m/s^2b 
    }

    return velocityMPS + ((totalAccel * MS_TIME_DELAY_MILSEC) / (1000)); // Multiply by 1000 to go from m/ms to m/s
}

void SendVelocityCANData(CANDATA_t outMsg){
    outMsg.ID = MOTOR_DRIVE;
    outMsg.idx = 0;

    memcpy(&outMsg.data[0], &velocityRPM, sizeof(velocityRPM));
    memcpy(&outMsg.data[4], &velocityMPS, sizeof(velocityMPS));

    CANbus_Send(outMsg, CAN_BLOCKING, MOTORCAN);
}

void Task1(void *arg)
{
    CANbus_Init(MOTORCAN);
    BSP_UART_Init(UART_2);

    while (1)
    {
        OS_ERR err;

        float setpointVelocity; 
        float setpointCurrent;
        float current; 
        float motorForce;
        
        CANDATA_t newMsg;
        CANDATA_t outMsg; // CANmsg we will be sending

        ErrorStatus error = CANbus_Read(&newMsg, CAN_NON_BLOCKING, MOTORCAN); // returns data value into newMsg 
        if (error == ERROR){ // If there is no new value to read, use the old CAN data
            newMsg = prevMsg;
        }

        // Parsing Measurements
        memcpy(&setpointVelocity, &newMsg.data[0], sizeof setpointVelocity); // in RPM 
        memcpy(&setpointCurrent, &newMsg.data[4], sizeof setpointCurrent); // Percent from 0.0 to 1.0
        
        if (abs(setpointVelocity) == MAX_VELOCITY){  // CURRENT CONTROLLED MODE

            current = (setpointVelocity < 0) ? -1.0f : 1.0f; // Calculated Current = 1 or -1

            motorForce = CurrentToMotorForce(current, setpointCurrent);

            velocityMPS = MotorForceToVelocity(motorForce);

            velocityRPM = MetersPerSecToRevPerMin(velocityMPS);

        } 
        else { // VELOCITY CONTROLLED MODE
            
            current = Velocity_PID_Output(setpointVelocity);

            motorForce = CurrentToMotorForce(current, setpointCurrent); 

            velocityMPS = MotorForceToVelocity(motorForce);

            velocityRPM = MetersPerSecToRevPerMin(velocityMPS);

        }


        ++cycle_ctr;
        if (cycle_ctr == 3)
        { // send outMsg to read canbus every 300 ms
            cycle_ctr = 0;
            // Return velocity as part of outMsg
            SendVelocityCANData(outMsg);
        }
        prevMsg = newMsg; // Update old CAN data to match most recent values received
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
