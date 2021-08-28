/* Copyright (c) 2020 UT Longhorn Racing Solar */

#include "common.h"
#include "config.h"
#include <bsp.h>
#include <unistd.h>
#include "MotorController.h"
#include "CarState.h"
#include "update_velocity.h"

int main(){
    MotorController_Init();
    car_state_t car_state;
    car_state.DesiredVelocity = MAX_VELOCITY; 
    car_state.DesiredMotorCurrent = 0.5;
    MotorController_Drive(velocity_to_rpm(car_state->CruiseControlVelocity), car_state->DesiredMotorCurrent)
    

    int size;

    size = 0;
    char str[128];
    uint8_t output;


    output = CANbus_Send(ids[0], payload);
}