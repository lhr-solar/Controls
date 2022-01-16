/* Copyright (c) 2020 UT Longhorn Racing Solar */

/**
 * Test file for the MotorCAN module of the controls board
 * 
 * Used by sending a CAN message on the MotorCAN module
 * which is then verified via a logic analyzer hooked up 
 * to the output and input headers to verify the driver code
 * and the hardware of the module is correct
 * 
 */

#include "common.h"
#include "config.h"
#include <bsp.h>
#include <unistd.h>
#include "MotorController.h"
#include "CarState.h"
#include "UpdateVelocity.h"
#include <math.h>

float velocity_to_rpm(float velocity)
{
    float velocity_mpm = velocity * 60.0f;
    float cir = WHEEL_DIAMETER * M_PI;
    float wheel_rpm = velocity_mpm / cir;
    return wheel_rpm * GEAR_REDUCTION;
}

int main()
{
    MotorController_Init();
    car_state_t car_state;
    car_state.DesiredVelocity = MAX_VELOCITY;
    car_state.DesiredMotorCurrent = 50;
    MotorController_Drive(velocity_to_rpm(car_state.DesiredVelocity), car_state.DesiredMotorCurrent);
}
