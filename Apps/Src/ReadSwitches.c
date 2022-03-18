/* Copyright (c) 2022 UT Longhorn Racing Solar */

// Includes
#include "ReadSwitches.h"

// Macros
#define MAX_SWITCH_LAG          100  // period (ms) for checking switches while precharging
#define PRECHARGE_DELAY_CYCLES  ((PRECHARGE_ARRAY_DELAY*1000) / MAX_SWITCH_LAG)

// Globals
OS_ERR err;

// Helper functions for reading/updating switches

static uint8_t UpdateSwitches();
static void UpdateLights();

// State functions

static void ElectronicsOn();
static void ArrayPrecharge();
static void ArrayOn();
static void MotorOn();

// States
typedef enum {
    UNUSED_SWITCH_NUM = 0,
    ELEC_ON = 0,
    ARRAY_PRE,
    ARRAY_ON,
    MOTOR_ON
} ignition_state_num_t;

typedef struct {
    /**
     * indexed by ignition switch read value (0, 1, or 3)
     *      0: electronics on only
     *      1: delay for array precharge -> array on
     *      3: motor on
     */
    ignition_state_num_t next_state[4];

    void (* statefunction)();   // function executed at state
    uint32_t delay_ms;          // ms delay after executing function, should be >= 1
} fsm_state_t;

fsm_state_t IgnitionFSM[] = {
    {{ELEC_ON, ARRAY_PRE,   UNUSED_SWITCH_NUM, ARRAY_PRE},  ElectronicsOn,  10}, // electronics on, array/motor off
    {{ELEC_ON, ARRAY_ON,    UNUSED_SWITCH_NUM, ARRAY_ON},   ArrayPrecharge, 10}, // array precharge, array/motor off
    {{ELEC_ON, ARRAY_ON,    UNUSED_SWITCH_NUM, MOTOR_ON},   ArrayOn,        10}, // array on, motor off
    {{ELEC_ON, ARRAY_ON,    UNUSED_SWITCH_NUM, MOTOR_ON},   MotorOn,        10}, // motor on, array on
};

/**
 * @brief Updates switch states and controls blinker/headlights accordingly;
 *        Also handles precharge/contactor logic with the ignition switch
 */
void Task_ReadSwitches(void* p_arg) {
    
    fsm_state_t *curr_fsm_state = &(IgnitionFSM[0]);

    // Initialization
    Switches_Init();
    Lights_Init();
    Contactors_Init();

    // Delay for precharge
    OSTimeDlyHMSM(0, 0, PRECHARGE_MOTOR_DELAY, 0, OS_OPT_TIME_HMSM_NON_STRICT, &err);

    // Main loop
    while (1) {
        curr_fsm_state->statefunction();
        curr_fsm_state = &(IgnitionFSM[curr_fsm_state->next_state[UpdateSwitches()]]);  // set next state
        OSTimeDlyHMSM(0, 0, 0, curr_fsm_state->delay_ms, OS_OPT_TIME_HMSM_NON_STRICT, &err);
    }
}

/**
 * @brief State where only electronics are on; contactors are all off
 */
static void ElectronicsOn() {
    Contactors_Set(ARRAY_CONTACTOR, OFF);
    Contactors_Set(ARRAY_PRECHARGE, OFF);
    Contactors_Set(MOTOR_CONTACTOR, OFF);
    return;
}

/**
 * @brief State for precharging the array before turning on. 
 *        Array will precharge for a time defined in "config.h"
 * @note  During the precharge delay, switches will be continually updated 
 *        with a period defined by MAX_SWITCH_LAG
 */
static void ArrayPrecharge() {
    Contactors_Set(ARRAY_PRECHARGE, ON);

    uint16_t i = 0;
    while ((i < PRECHARGE_DELAY_CYCLES) && (UpdateSwitches() != 0)) {
        OSTimeDlyHMSM(0, 0, 0, MAX_SWITCH_LAG, OS_OPT_TIME_HMSM_NON_STRICT, &err);
        i++;
    }
}

/**
 * @brief Array On state. Motor contactor should be off here.
 */
static void ArrayOn() {
    Contactors_Set(MOTOR_CONTACTOR, OFF);
    Contactors_Set(ARRAY_CONTACTOR, ON);
    Contactors_Set(ARRAY_PRECHARGE, OFF);
}

/**
 * @brief Motor On state. All non-prechage contactors - 
 *        (Array contactor, Motor contactor) are on.
 */
static void MotorOn() {
    Contactors_Set(MOTOR_CONTACTOR, ON);
}

/**
 * @brief Helper function for updating switches and lights. Updated switch 
 *        values are stored locally in the switches driver.
 * 
 * @return uint8_t 2-bit ignition switch value
 */
static uint8_t UpdateSwitches() {
    Switches_UpdateStates();

    UpdateLights();

    return (Switches_Read(IGN_2) << 1) + Switches_Read(IGN_1);
}

/**
 * @brief Helper function for updating lights with switch values.
 */
static void UpdateLights() {
    Lights_Set(Headlight_ON, Switches_Read(HEADLIGHT_SW));
    
    int leftblink = Switches_Read(LEFT_SW) | 
                    Switches_Read(HZD_SW);
    int rightblink = Switches_Read(RIGHT_SW) | 
                     Switches_Read(HZD_SW);

    Toggle_Set(RIGHT_BLINK, rightblink);
    Toggle_Set(LEFT_BLINK, leftblink);
}