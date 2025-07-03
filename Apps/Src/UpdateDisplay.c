/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file UpdateDisplay.c
 * @brief Function implementations for the display application.
 *
 * This contains functions relevant to modifying states of specific
 * components on our HMI design. The HMI has the ability to indicate
 * relevant information about system status to the driver.
 *
 */

#include "os_cfg_app.h"

#include "Contactors.h"
#include "DebugIO.h"
#include "Display.h"

#include "Tasks.h"
#include "UpdateDisplay.h"

bool leftBlinkerOn = false;
bool rightBlinkerOn = false;

#define MOT_LIMIT_LEN 15

static char motLimit[MOT_LIMIT_LEN] = "\"NONE\"";

#define UPDATE_DISPLAY_DELAY 250
#define BLINKER_COUNT (500)/UPDATE_DISPLAY_DELAY

// For fault handling
#define RESTART_THRESHOLD 3 // number of times to reset before displaying the fault screen

controls_error_e UpdateDisplay_Init() {
    OS_ERR err;
    DisplayError_t ret = Display_SetPage(INFO);
    OSTimeDlyHMSM(0, 0, 0, 450, OS_OPT_TIME_HMSM_STRICT, &err);
    assertOSError(err);
    leftBlinkerOn = false;
    rightBlinkerOn = false;

    return (ret == DISPLAY_ERR_NONE) ? C_ERR_NONE : C_ERR_UPD_DRIVER;
}

/**
 * @brief Uses component enum to make assigning component values easier.
 * Differentiates between timers, variables, and components to assign values.
 * @param comp component to set value of
 * @param val value
 * @return controls_error_e
 */
static controls_error_e UpdateDisplay_SetComponent(Component_t comp) {
    DisplayError_t ret = DISPLAY_ERR_NONE;
    // For components that are on/off
    if (comp <= DISP_LEFT_BLINK) {
        if (g_display_comp_modified[comp]) {
            DisplayCmd_t visCmd = {
                .compOrCmd = "vis",
                .attr = NULL,
                .op = NULL,
                .numArgs = 2,
                .argTypes = {STR_ARG, INT_ARG},
                .args = {{.str = (char *)DISPLAY_COMP_STR[comp]}, {.num = g_display_comp_vals[comp]}}
            };
            ret = Display_Send(visCmd);
            if (ret == DISPLAY_ERR_NONE) g_display_comp_modified[comp] = false;
        }
    }
    // For components that have a non-boolean value but aren't the contactors
    else if (comp < DISP_MOT_LIMIT) {
        uint32_t comp_val = 0;
        if (comp > DISP_MOTOR_PC) {
            comp_val = g_display_comp_vals[comp];
        } else {
            bool state;
            switch (comp) {
                case DISP_ARRAY_EN:
                    state = Contactors_Get(ARRAY_CONTACTOR, false);
                    if (comp_val != state) {
                        g_display_comp_modified[comp] = true;
                        comp_val = state;
                    }
                    break;
                case DISP_ARRAY_PC:
                    state = Contactors_Get(ARRAY_PRECHARGE_BYPASS_CONTACTOR, false);
                    if (comp_val != state) {
                        g_display_comp_modified[comp] = true;
                        comp_val = state;
                    }
                    break;
                case DISP_MOTOR_EN:
                    state = Contactors_Get(MOTOR_CONTROLLER_CONTACTOR, false);
                    if (comp_val != state) {
                        g_display_comp_modified[comp] = true;
                        comp_val = state;
                    }
                    break;
                case DISP_MOTOR_PC:
                    state = Contactors_Get(MOTOR_CONTROLLER_PRECHARGE_BYPASS_CONTACTOR, false);
                    if (comp_val != state) {
                        g_display_comp_modified[comp] = true;
                        comp_val = state;
                    }
                    break;
                default:
                    break;
            }
        }

        // Only send display cmd if its been changed since last update
        if (g_display_comp_modified[comp]) {
            DisplayCmd_t setCmd = {
                .compOrCmd = (char *)DISPLAY_COMP_STR[comp],
                .attr = "val",
                .op = "=",
                .numArgs = 1,
                .argTypes = {INT_ARG},
                .args = {{.num = comp_val}}
            };
            ret = Display_Send(setCmd);
            if (ret == DISPLAY_ERR_NONE) g_display_comp_modified[comp] = false;
        }       
    } else if (comp == DISP_MOT_LIMIT) {
        // Only send display cmd if its been changed since last update
        if (g_display_comp_modified[comp]) {
            DisplayCmd_t setCmd = {
                .compOrCmd = (char *)DISPLAY_COMP_STR[comp],
                .attr = "txt",
                .op = "=",
                .numArgs = 1,
                .argTypes = {STR_ARG},
                .args = {{.str = motLimit}}
            };
            ret = Display_Send(setCmd);
            if (ret == DISPLAY_ERR_NONE) g_display_comp_modified[comp] = false;
        }  
    }

    return (ret == DISPLAY_ERR_NONE) ? C_ERR_NONE : C_ERR_UPD_DRIVER;
}

/* WRAPPERS */
controls_error_e UpdateDisplay_SetSOC(uint32_t percent) { // Integer percentage from 0-100
    if (percent > 100) {
        return C_ERR_UPD_PARSE_COMPONENT;
    }

    g_display_comp_vals[DISP_SOC] = percent;
    g_display_comp_modified[DISP_SOC] = true;
    return C_ERR_NONE;
}

controls_error_e UpdateDisplay_SetMotorLimit(uint16_t motor_limit_flag) {
    if (motor_limit_flag == 0) {
        strncpy(motLimit, "\"NONE\"", MOT_LIMIT_LEN);
    }
    else if ((motor_limit_flag & (motor_limit_flag - 1))) {
        strncpy(motLimit, "\"MULT_LIMIT\"", MOT_LIMIT_LEN);
    }
    else if (motor_limit_flag & 0x01) {
        strncpy(motLimit, "\"OUTPUT_PWM\"", MOT_LIMIT_LEN);
    }
    else if ((motor_limit_flag >> 1) & 0x01) {
        strncpy(motLimit, "\"MOTOR_CURR\"", MOT_LIMIT_LEN);
    }
    else if ((motor_limit_flag >> 2) & 0x01) {
        strncpy(motLimit, "\"VELOCITY\"", MOT_LIMIT_LEN);
    }
    else if ((motor_limit_flag >> 3) & 0x01) {
        strncpy(motLimit, "\"BUS_CURR\"", MOT_LIMIT_LEN);
    }
    else if ((motor_limit_flag >> 4) & 0x01) {
        strncpy(motLimit, "\"BUS_V_UPPER\"", MOT_LIMIT_LEN);
    }
    else if ((motor_limit_flag >> 5) & 0x01) {
        strncpy(motLimit, "\"BUS_V_LOWER\"", MOT_LIMIT_LEN);
    }
    else if ((motor_limit_flag >> 6) & 0x01) {
        strncpy(motLimit, "\"TEMPERATURE\"", MOT_LIMIT_LEN);
    }

    return C_ERR_NONE;
}

controls_error_e UpdateDisplay_SetSBPV(uint32_t mv) {
    g_display_comp_vals[DISP_SUPP_BATT] = (mv / 100); // mv to tenths of a volt;
    g_display_comp_modified[DISP_SUPP_BATT] = true;
    return C_ERR_NONE;
}

controls_error_e UpdateDisplay_SetVelocity(uint32_t mphTenths) {
    g_display_comp_vals[DISP_VELOCITY] = mphTenths;
    g_display_comp_modified[DISP_VELOCITY] = true;
    return C_ERR_NONE;
}

controls_error_e UpdateDisplay_SetAccel(uint8_t percent) {
    g_display_comp_vals[DISP_ACCEL_METER] = (uint32_t)((percent > 100) ? 100 : percent);
    g_display_comp_modified[DISP_ACCEL_METER] = true;
    return C_ERR_NONE;
}

controls_error_e UpdateDisplay_SetBlink(bool leftState, bool rightState) {
    leftBlinkerOn = leftState;
    rightBlinkerOn = rightState;

    return C_ERR_NONE;
}

controls_error_e UpdateDisplay_SetGear(TriState_t gear) {
    g_display_comp_vals[DISP_GEAR] = (uint32_t)(gear);
    g_display_comp_modified[DISP_GEAR] = true;
    return C_ERR_NONE;
}

// controls_error_e UpdateDisplay_SetRegenState(TriState_t state) {
//     g_display_comp_vals[DISP_REGEN_ST] = (uint32_t)(state);
//     g_display_comp_modified[DISP_REGEN_ST] = true;
//     return C_ERR_NONE;
// }

// controls_error_e UpdateDisplay_SetCruiseState(TriState_t state) {
//     g_display_comp_vals[DISP_CRUISE_ST] = (uint32_t)(state);
//     g_display_comp_modified[DISP_CRUISE_ST] = true;
//     return C_ERR_NONE;
// }

controls_error_e UpdateDisplay_SetBattVoltage(uint32_t mv) {
    g_display_comp_vals[DISP_PACK_VOLTAGE] = (mv / 100); // mv to tenths of a volt
    g_display_comp_modified[DISP_PACK_VOLTAGE] = true;
    return C_ERR_NONE;
}

controls_error_e UpdateDisplay_SetBattTemperature(uint32_t val) {
    g_display_comp_vals[DISP_PACK_TEMP] = (val / 100);
    g_display_comp_modified[DISP_PACK_TEMP] = true;
    return C_ERR_NONE;
}

controls_error_e UpdateDisplay_SetBattCurrent(int32_t val) {
    bool is_neg = (val < 0);
    g_display_comp_vals[DISP_PACK_CURR_SIGN] = (uint32_t)(is_neg);
    g_display_comp_vals[DISP_PACK_CURRENT] = ((uint32_t)(is_neg ? -val : val) / 100);
    g_display_comp_modified[DISP_PACK_CURR_SIGN] = true;
    g_display_comp_modified[DISP_PACK_CURRENT] = true;
    return C_ERR_NONE;
}

controls_error_e UpdateDisplay_SetMCVoltage(uint32_t volts) {
    g_display_comp_vals[DISP_MC_BUS_VOLTAGE] = volts;
    g_display_comp_modified[DISP_MC_BUS_VOLTAGE] = true;
    return C_ERR_NONE;
}

controls_error_e UpdateDisplay_SetMCCurrent(int32_t val) {
    bool is_neg = (val < 0);
    g_display_comp_vals[DISP_MC_CURR_SIGN] = (uint32_t)(is_neg);
    g_display_comp_vals[DISP_MC_BUS_CURRENT] = (uint32_t)(is_neg ? -val : val);
    g_display_comp_modified[DISP_MC_CURR_SIGN] = true;
    g_display_comp_modified[DISP_MC_BUS_CURRENT] = true;
    return C_ERR_NONE;
}

controls_error_e UpdateDisplay_SetBrake(bool state) {
    g_display_comp_vals[DISP_BRAKE] = state;
    g_display_comp_modified[DISP_BRAKE] = true;
    return C_ERR_NONE;
}

controls_error_e UpdateDisplay_SetHeartbeat(bool val) {
    g_display_comp_vals[DISP_HEARTBEAT] = (uint32_t)(val);
    g_display_comp_modified[DISP_HEARTBEAT] = true;
    return C_ERR_NONE;
}

controls_error_e UpdateDisplay_SetHeatSinkTemp(uint32_t val) {
    g_display_comp_vals[DISP_HEAT_SINK_TEMP] = val;
    g_display_comp_modified[DISP_HEAT_SINK_TEMP] = true;
    return C_ERR_NONE;
}


static inline void Update_Blinkers() {
    static uint8_t blinkCounter = 0;
    
    // Blink the left and right indicators about once every second
    if (blinkCounter >= BLINKER_COUNT) {
        if (rightBlinkerOn) {
            g_display_comp_vals[DISP_RIGHT_BLINK] = g_display_comp_vals[DISP_RIGHT_BLINK] ? 0 : 1;
        } else {
            g_display_comp_vals[DISP_RIGHT_BLINK] = 0;
        }
        g_display_comp_modified[DISP_RIGHT_BLINK] = true;

        if (leftBlinkerOn) {
            g_display_comp_vals[DISP_LEFT_BLINK] = g_display_comp_vals[DISP_LEFT_BLINK] ? 0 : 1;
        } else {
            g_display_comp_vals[DISP_LEFT_BLINK] = 0;
        }
        g_display_comp_modified[DISP_LEFT_BLINK] = true;

        blinkCounter = 0;
    }
    else {
        blinkCounter++;
    }

}
/**
 * @brief Loops through the display queue and sends all messages
 */
void Task_UpdateDisplay(void *p_arg) {
    OS_ERR err;
    while (1) {
#ifdef TASK_PROFILER
        DebugIO_Toggle(UPDATE_DISPLAY_PIN);
#endif
        Component_t comp;

        /*   ALL BOOLEANS   */
        UpdateDisplay_SetHeartbeat(g_display_comp_vals[DISP_HEARTBEAT] ? 0 : 1);
        for (comp = 0; comp <= DISP_LEFT_BLINK; comp++) {
            assertUpdateDisplayError(UpdateDisplay_SetComponent(comp));
        }

        // Refresh needed for bool values
        assertUpdateDisplayError((Display_Refresh() == DISPLAY_ERR_NONE) ? C_ERR_NONE
                                                                         : C_ERR_UPD_DRIVER);

        Update_Blinkers();

        OSTimeDlyHMSM(0, 0, 0, 5, OS_OPT_TIME_HMSM_STRICT, &err);
        assertOSError(err);

        /*   ALL CONTACTORS   */
        for (comp = DISP_ARRAY_EN; comp <= DISP_MOTOR_PC; comp++) {
            assertUpdateDisplayError(UpdateDisplay_SetComponent(comp));
        }

        OSTimeDlyHMSM(0, 0, 0, 5, OS_OPT_TIME_HMSM_STRICT, &err);
        assertOSError(err);

        /*   HALF OF VAL TYPE COMPONENTS   */
        for (comp = DISP_VELOCITY; comp <= DISP_MOTOR_PC; comp++) {
            assertUpdateDisplayError(UpdateDisplay_SetComponent(comp));
        }

        OSTimeDlyHMSM(0, 0, 0, 5, OS_OPT_TIME_HMSM_STRICT, &err);
        assertOSError(err);

        /*   OTHER HALF OF VAL TYPE COMPONENTS   */
        for (comp = DISP_PACK_CURRENT; comp <= DISP_MOT_LIMIT; comp++) {
            assertUpdateDisplayError(UpdateDisplay_SetComponent(comp));
        }

        // Refresh needed for gear
        assertUpdateDisplayError((Display_Refresh() == DISPLAY_ERR_NONE) ? C_ERR_NONE
                                                                         : C_ERR_UPD_DRIVER);

#ifdef TASK_PROFILER
        DebugIO_Toggle(UPDATE_DISPLAY_PIN);
#endif

        OSTimeDlyHMSM(0, 0, 0, UPDATE_DISPLAY_DELAY, OS_OPT_TIME_HMSM_STRICT, &err);
        assertOSError(err);
    }
}

/**
 * Error handler functions
 * Passed as callback functions to the main throwTaskError function by assertUpdateDisplayError
 */

/**
 * @brief A handler callback function run by the main throwTaskError function
 * used if we haven't reached the restart limit and encounter an error
 */
void handler_UpdateDisplay_Restart() {
    Display_Reset(); // Try resetting to fix the display error
}

/**
 * @brief Check for a display error and assert it if it exists.
 * Stores the error code, calls the main assertion function
 * and runs a callback function as a handler to restart the display and clear the queue.
 * @param   err variable with display error codes
 */
void assertUpdateDisplayError(controls_error_e uderr) {
    switch (uderr) {
        case C_ERR_NONE:
            break;
        case C_ERR_UPD_GENERIC:
        case C_ERR_UPD_PARSE_COMPONENT:
        case C_ERR_UPD_DRIVER:
            throwTaskError(uderr, false, handler_UpdateDisplay_Restart, OPT_NO_LOCK_SCHED,
                           OPT_NONRECOV, CAN_NONE_BPS);
            break;
        default:
            // Critical failure, we have a non updatedisplay error in updatedisplay somehow
            throwTaskError(C_ERR_ILLEGAL_ERROR, false, NULL, OPT_LOCK_SCHED, OPT_NONRECOV, CAN_NONE_BPS);
            break;
    }
}
