/* Copyright (c) 2022 UT Longhorn Racing Solar */

// Includes
#include "ReadSwitches.h"

// Macros
#define MAX_SWITCH_LAG          100  // period (ms) for checking switches while precharging
#define READ_SWITCH_PERIOD      20   // period (ms) that switches will be read.
#define PRECHARGE_DELAY_CYCLES  ((PRECHARGE_ARRAY_DELAY*1000) / MAX_SWITCH_LAG)

// Globals
OS_ERR err;

// Helper functions for reading/updating switches

static uint8_t UpdateSwitches();
static void UpdateLights();

// State functions

static int ElectronicsOn();
static int ArrayPrecharge();
static int ArrayOn();
static int ArrayEn();
static int MotorOn();
static int MotorEn();

// States
typedef enum {
    UNUSED_SWITCH_NUM = 0,
    ELEC_ON = 0,
    ARRAY_PRE,
    ARRAY_ON,
    ARRAY_EN,
    MOTOR_ON,
    MOTOR_EN
} ignition_state_num_t;

typedef struct {
    /**
     * indexed by ignition switch read value (0, 1, or 3)
     *      0: electronics on only (all contactors disabled)
     *      1: delay for array precharge -> array contactor on -> array contactor enable
     *      3: motor contactor on -> motor contactor enable
     */
    ignition_state_num_t next_state[4];

    int (* statefunction)();   // function executed at state
    uint32_t delay_ms;          // ms delay after executing function, should be >= 1
} fsm_state_t;

fsm_state_t IgnitionFSM[] = {
    {{ELEC_ON, ARRAY_PRE, UNUSED_SWITCH_NUM, ARRAY_PRE}, ElectronicsOn,  READ_SWITCH_PERIOD}, /* electronics on, array/motor disable */
    {{ELEC_ON, ARRAY_ON,  UNUSED_SWITCH_NUM, ARRAY_ON},  ArrayPrecharge, READ_SWITCH_PERIOD}, /* array precharge, array/motor disable */
    {{ELEC_ON, ARRAY_EN,  UNUSED_SWITCH_NUM, ARRAY_EN},  ArrayOn,        READ_SWITCH_PERIOD}, /* array on, motor off */
    {{ELEC_ON, ARRAY_EN,  UNUSED_SWITCH_NUM, MOTOR_ON},  ArrayEn,        READ_SWITCH_PERIOD}, /* array on, motor off */
    {{ELEC_ON, ARRAY_EN,  UNUSED_SWITCH_NUM, MOTOR_EN},  MotorOn,        READ_SWITCH_PERIOD}, /* motor on, array on */
    {{ELEC_ON, ARRAY_EN,  UNUSED_SWITCH_NUM, MOTOR_EN},  MotorEn,        READ_SWITCH_PERIOD}, /* array on, motor off */
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
        int status = curr_fsm_state->statefunction();
        
        /*
         * if the precharge wait exits early then status == 1; 
         * we don't update switches again in case the driver 
         * toggles the switch to position 1 or 3 between returning from
         * ArrayPrecharge() and setting the next state
         */

        uint8_t ignition_pos = status ? 0 : UpdateSwitches();
        curr_fsm_state = &(IgnitionFSM[curr_fsm_state->next_state[ignition_pos]]);  // set next state
        OSTimeDlyHMSM(0, 0, 0, curr_fsm_state->delay_ms, OS_OPT_TIME_HMSM_NON_STRICT, &err);
    }
}

/**
 * @brief State where only electronics are on; contactors are all disabled and off
 * 
 * @return Always returns 0.
 */
static int ElectronicsOn() {
    Contactors_Disable(ARRAY_CONTACTOR);
    Contactors_Disable(ARRAY_PRECHARGE);
    Contactors_Disable(MOTOR_CONTACTOR);
    return 0;
}

/**
 * @brief State for precharging the array before turning on. 
 *        Array will precharge for a time defined in "config.h"
 *        The precharge contactor is enabled and set here.
 * @note  During the precharge delay, switches will be continually updated 
 *        with a period defined by MAX_SWITCH_LAG
 * 
 * @return 0 if precharge fully completed, 1 if exited early
 */
static int ArrayPrecharge() {
    Contactors_Set(ARRAY_PRECHARGE, ON);
    Contactors_Enable(ARRAY_PRECHARGE);

    for (uint16_t i = 0; i < PRECHARGE_DELAY_CYCLES; i++) {
        if (UpdateSwitches() == 0) return 1;
        OSTimeDlyHMSM(0, 0, 0, MAX_SWITCH_LAG, OS_OPT_TIME_HMSM_NON_STRICT, &err);
    }
    return 0;
}

/**
 * @brief Array On state. Motor contactor should be off and disabled here.
 * @note Array contactor is disabled here (not yet enabled)
 * 
 * @return Always returns 0.
 */
static int ArrayOn() {
    Contactors_Set(ARRAY_CONTACTOR, ON);
    Contactors_Set(ARRAY_PRECHARGE, OFF);
    return 0;
}

/**
 * @brief Array Enable state. Motor contactor should be off and disabled here.
 * @note Does not change the state of the array contactor, only enables
 * 
 * @return Always returns 0.
 */
static int ArrayEn() {
    Contactors_Disable(MOTOR_CONTACTOR);
    Contactors_Enable(ARRAY_CONTACTOR);
    return 0;
}

/**
 * @brief Motor On state.
 * @note Motor contactor is disabled here (not yet enabled)
 * 
 * @return Always returns 0.
 */
static int MotorOn() {
    Contactors_Set(MOTOR_CONTACTOR, ON);
    return 0;
}

/**
 * @brief Motor Enable state. All contactors - 
 *        (Array contactor, Motor contactor) are enabled.
 * 
 * @return Always returns 0.
 */
static int MotorEn() {
    Contactors_Enable(MOTOR_CONTACTOR);
    return 0;
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