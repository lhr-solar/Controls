/* Copyright (c) 2022 UT Longhorn Racing Solar */

// Includes
#include "os.h"
#include "common.h"
#include "config.h"
#include "Tasks.h"
#include "MotorController.h"
#include "Contactors.h"
#include "Minions.h"
#include "UpdateDisplay.h"

// Macros
#define READ_SWITCH_PERIOD      10   // period (ms) that switches will be read.
//static bool restartNeeded = true;

// Helper functions for reading/updating switches

static void UpdateSwitches();
//static void UpdateLights();
static void UpdateDispLights();

/**
 * @brief Updates switch states and controls blinker/headlights accordingly;
 *        Also handles precharge/contactor logic with the ignition switch
 */
void Task_ReadSwitches(void* p_arg) {
    OS_ERR err;

    // Delay for precharge
    OSTimeDlyHMSM(0, 0, PRECHARGE_MOTOR_DELAY, 0, OS_OPT_TIME_HMSM_NON_STRICT, &err);
    assertOSError(OS_SWITCHES_LOC, err);

    Contactors_Enable(MOTOR_CONTACTOR);
    static bool lastCruiseEnPushed = true; //cruise control edge detector variables
    static State cruiseEnablePushed = OFF;
    // Main loop
    while (1) {

        UpdateSwitches();

        // array on/off
        if (Switches_Read(IGN_1) == ON || Switches_Read(IGN_2) == ON) {
            Contactors_Enable(ARRAY_CONTACTOR);
            Contactors_Enable(ARRAY_PRECHARGE);
            Lights_Set(A_CNCTR,ON);
            UpdateDisplay_SetArray(true);
        } else {
            Contactors_Disable(ARRAY_CONTACTOR);
            Contactors_Disable(ARRAY_PRECHARGE);
            Lights_Set(A_CNCTR,OFF);
            UpdateDisplay_SetArray(false);
        }
        
        // motor on/off
        Contactors_Set(MOTOR_CONTACTOR, Switches_Read(IGN_2));
        // if(Contactors_Get(MOTOR_CONTACTOR) && restartNeeded){ //restart contactor whenever it is on and it wasn't on earlier
        //     MotorController_Restart();
        //     restartNeeded = false;
        // } else if (!Contactors_Get(MOTOR_CONTACTOR)) { //if contactor is off, set restartNeeded
        //     restartNeeded = true;
        // }
        Lights_Set(M_CNCTR,Switches_Read(IGN_2));

        cruiseEnablePushed = Switches_Read(CRUZ_EN); //read cruise enable switch
        if (!lastCruiseEnPushed && cruiseEnablePushed) { //Rising Edge detector for updateVelocity thread
            UpdateVel_ToggleCruise = true; //updateVelocity thread depends on this
        }
        
        lastCruiseEnPushed = cruiseEnablePushed;


        OSTimeDlyHMSM(0, 0, 0, READ_SWITCH_PERIOD, OS_OPT_TIME_HMSM_NON_STRICT, &err);
        assertOSError(OS_SWITCHES_LOC, err);
    }
}

/**
 * @brief Helper function for updating switches and lights. Updated switch 
 *        values are stored locally in the switches driver.
 * 
 * @return none
 */
static void UpdateSwitches() {
    Switches_UpdateStates();

    //UpdateLights();
    UpdateDispLights();
}

/**
 * @brief Helper function for updating lights with switch values.
 */
/*static void UpdateLights() {
    Lights_Set(Headlight_ON, Switches_Read(HEADLIGHT_SW));
    
    int leftblink = Switches_Read(LEFT_SW) | 
                    Switches_Read(HZD_SW);
    int rightblink = Switches_Read(RIGHT_SW) | 
                     Switches_Read(HZD_SW);
    if((rightblink && leftblink) && (Lights_Read(LEFT_BLINK)!=Lights_Read(RIGHT_BLINK))){ //hazards are on and we are desynced
        Lights_Toggle_Set(RIGHT_BLINK, OFF);
        Lights_Toggle_Set(LEFT_BLINK, OFF);
        Lights_Set(RIGHT_BLINK,OFF);
        Lights_Set(LEFT_BLINK,OFF);
    }
    Lights_Toggle_Set(RIGHT_BLINK, rightblink);
    Lights_Toggle_Set(LEFT_BLINK, leftblink);
    if(leftblink==0){
        Lights_Set(LEFT_BLINK,OFF);
    }
    if(rightblink==0){
        Lights_Set(RIGHT_BLINK,OFF);
    }
    
}*/

/**
 * @brief Helper function for updating lights with switch values.
 */
static void UpdateDispLights() {
    Lights_Set(Headlight_ON, Switches_Read(HEADLIGHT_SW));
    UpdateDisplay_SetHeadlight(Switches_Read(HEADLIGHT_SW));
    
    int leftblink = Switches_Read(LEFT_SW) | 
                    Switches_Read(HZD_SW);
    int rightblink = Switches_Read(RIGHT_SW) | 
                     Switches_Read(HZD_SW);
    
    if((rightblink && leftblink) && (Lights_Read(LEFT_BLINK)!=Lights_Read(RIGHT_BLINK))){ //hazards are on and we are desynced
        Lights_Toggle_Set(RIGHT_BLINK, OFF);
        Lights_Toggle_Set(LEFT_BLINK, OFF);
        Lights_Set(RIGHT_BLINK,OFF);
        Lights_Set(LEFT_BLINK,OFF);
        
        UpdateDisplay_SetLeftBlink(false);
        UpdateDisplay_SetRightBlink(false);
    }
    Lights_Toggle_Set(RIGHT_BLINK, rightblink);
    Lights_Toggle_Set(LEFT_BLINK, leftblink);
    
    UpdateDisplay_SetLeftBlink(leftblink);
    UpdateDisplay_SetRightBlink(rightblink);
    
    if(leftblink==0){
        Lights_Set(LEFT_BLINK,OFF);
        UpdateDisplay_SetLeftBlink(false);
    }
    if(rightblink==0){
        Lights_Set(RIGHT_BLINK,OFF);
        UpdateDisplay_SetRightBlink(false);
    }
    
}