/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file Display.c
 * @brief Function implementations for the display driver.
 *
 * This contains functions relevant to sending/receiving messages
 * to/from our Nextion display.
 *
 */

#include "Display.h"
#include "Tasks.h" // for os and fault error codes
#include "bsp.h"   // for writing to UART
#include "IOState.h"
#include "ReadCarCAN.h"
#include "ReadTritium.h"
#include "UpdateDisplay.h"
#include "SendTritium.h"

// Assignment commands have only 1 arg, an operator, and an attribute
#define IS_ASSIGN_CMD(cmd)                              \
    (cmd.compOrCmd != NULL && cmd.op != NULL &&         \
     cmd.attr != NULL && cmd.numArgs == 1)
// Operational commands have no attribute and no operator, just a command and
// >= 0 arguments
#define IS_OP_CMD(cmd) (cmd.op == NULL && cmd.attr == NULL)

static const char *TERMINATOR = "\xff\xff\xff";

// Hold component values for display
uint32_t g_display_comp_vals[DISP_NUM_COMPONENTS] = {0};

// Strings for each component id
const char *DISPLAY_COMP_STR[DISP_NUM_COMPONENTS] = {
    // Boolean components
    "hb", "cs", "mcs", "brake", "blink",
    // Contactors
    "arren", "arrpc", "moten", "motpc", // technically boolean but the logic is cooked
    // Non-boolean components
    "vel", "accel", "soc", "supp", "cruiseSt", "rbsSt", "pv", "pc", "pt", "mcv",
    "mcc", "heatsink", "gear",
    // Fault code components
    "oserr", "faulterr", "evac"
};


/**
 * @brief Initializes the display driver
 * @returns DisplayError_t
 */
DisplayError_t Display_Init() {
    BSP_UART_Init(DISPLAY);
    return Display_Reset();
}

/**
 * @brief Sends a command to the display
 * @param cmd command to send
 * @returns DisplayError_t
 */
DisplayError_t Display_Send(DisplayCmd_t cmd) {
    char msgArgs[MAX_MSG_LEN] = {0}; // Initialize to avoid garbage values

    if (IS_ASSIGN_CMD(cmd)) {
        if (cmd.argTypes[0] == INT_ARG) {
            sprintf(msgArgs, "%d", (int) cmd.args[0].num);
        } else { // STR_ARG
            if (cmd.args[0].str == NULL)
            return DISPLAY_ERR_PARSE;
            sprintf(msgArgs, "%s", cmd.args[0].str);
        }

        BSP_UART_Write(DISPLAY, cmd.compOrCmd, strlen(cmd.compOrCmd));
        BSP_UART_Write(DISPLAY, ".", 1);
        BSP_UART_Write(DISPLAY, cmd.attr, strlen(cmd.attr));
        BSP_UART_Write(DISPLAY, cmd.op, strlen(cmd.op));
    } else if (IS_OP_CMD(cmd)) {
        if (cmd.numArgs > MAX_ARGS) return DISPLAY_ERR_OTHER;

        msgArgs[0] = ' '; // No args
        msgArgs[1] = '\0';
        if (cmd.numArgs >= 1) { // If there are arguments
            for (int i = 0; i < cmd.numArgs; i++) {
                char arg[MAX_ARG_LEN];
                if (cmd.argTypes[i] == INT_ARG) {
                    sprintf(arg, "%d", (int) cmd.args[i].num);
                } else { // STR_ARG
                    if (cmd.args[i].str == NULL) return DISPLAY_ERR_PARSE;
                    sprintf(arg, "%s", cmd.args[i].str);
                }

                strcat(msgArgs, arg);

                // Add delimiter
                if (i < cmd.numArgs - 1) strcat(msgArgs, ",");
            }
        }

        BSP_UART_Write(DISPLAY, cmd.compOrCmd, strlen(cmd.compOrCmd));
    } 
    else { // Error parsing command struct
        return DISPLAY_ERR_PARSE;
    }

    // If there are arguments, write them
    if (cmd.numArgs >= 1) { 
        BSP_UART_Write(DISPLAY, msgArgs, strlen(msgArgs));
    }

    BSP_UART_Write(DISPLAY, (char *)TERMINATOR, strlen(TERMINATOR));

    return DISPLAY_ERR_NONE;
}

/**
 * @brief Resets the display
 * @returns DisplayError_t
 */
DisplayError_t Display_Reset() {
    DisplayCmd_t restCmd = {
        .compOrCmd = "rest", 
        .attr = NULL, 
        .op = NULL, 
        .numArgs = 0
    };

    // Terminates any in progress command
    BSP_UART_Write(DISPLAY, (char *)TERMINATOR, strlen(TERMINATOR));

    return Display_Send(restCmd);
}

/**
 * @brief Several elements on the display do not update their
 * state until a touch/click event is triggered. This includes the
 * blinkers, gear selector, cruise control and regen braking indicator.
 * @returns DisplayError_t
 */
DisplayError_t Display_Refresh() {
	DisplayCmd_t refreshCmd = {
		.compOrCmd = "click",
		.attr = NULL,
		.op = NULL,
		.numArgs = 2,
		.argTypes = {INT_ARG,INT_ARG},
		.args = {
			{.num = 0},
			{.num = 1}
		}
	};

	return Display_Send(refreshCmd);
}

/**
 * @brief Changes the page of the display
 * @returns DisplayError_t
 */
DisplayError_t Display_SetPage(Page_t page) {
	DisplayCmd_t pgCmd = {
		.compOrCmd = "page",
		.attr = NULL,
		.op = NULL,
		.numArgs = 1,
		.argTypes = {INT_ARG},
		.args = {{.num = page}}
	};

	return Display_Send(pgCmd);
}

/**
 * @brief Overwrites any processing commands and triggers the display fault screen
 * @returns DisplayError_t
 */
DisplayError_t Display_Error() {
    // Terminates any in progress command
    BSP_UART_Write(DISPLAY, (char*) TERMINATOR, strlen(TERMINATOR)); 

    // Switch to fault page
    Display_SetPage(FAULT);

    DisplayCmd_t evac_msg_cmd = {
        .compOrCmd = (char*) DISPLAY_COMP_STR[DISP_EVAC_MSG], // "evac"
        .attr = "txt",
        .op = "=",
        .numArgs = 1,
        .argTypes = {STR_ARG},
        .args = {{.str = ErrMsg_Evac}}
    };
    Display_Send(evac_msg_cmd);
    strncpy(ErrMsg_Evac, DISP_ERRMSG_NA, ERR_CODE_LEN);

    // Display OS error if there is one
    DisplayCmd_t os_flt_cmd = {
        .compOrCmd = (char*) DISPLAY_COMP_STR[DISP_OS_CODE], // "oserr"
        .attr = "txt",
        .op = "=",
        .numArgs = 1,
        .argTypes = {STR_ARG},
        .args = {{.str = ErrMsg_OS}}
    };
    Display_Send(os_flt_cmd);
    strncpy(ErrMsg_OS, DISP_ERRMSG_NA, ERR_CODE_LEN);
    memset(&Error_OS, 0, sizeof(error_code_t));

    // Display other errors if there are any.
    // Prioritized errors in order of importance:
    // 1. ReadTritium
    // 2. ReadCarCAN
    // 3. UpdateDisplay
    // 4. SendTritium
    // 5. IOState
    if (Error_ReadTritium != T_NONE) {
        DisplayCmd_t moco_flt_cmd = {
            .compOrCmd = (char*) DISPLAY_COMP_STR[DISP_FAULT_CODE], // "faulterr"
            .attr = "txt",
            .op = "=",
            .numArgs = 1,
            .argTypes = {STR_ARG},
            .args = {{.str = ErrMsg_ReadTritium}}
        };
        Display_Send(moco_flt_cmd);
        strncpy(ErrMsg_ReadTritium, DISP_ERRMSG_NA, ERR_CODE_LEN);
        memset(&Error_ReadTritium, 0, sizeof(error_code_t));
    } else if (Error_SendTritium != SENDTRITIUM_ERR_NONE) {
        DisplayCmd_t disp_flt_cmd = {
            .compOrCmd = (char*) DISPLAY_COMP_STR[DISP_FAULT_CODE], // "faulterr"
            .attr = "txt",
            .op = "=",
            .numArgs = 1,
            .argTypes = {STR_ARG},
            .args = {{.str = ErrMsg_SendTritium}}
        };
        Display_Send(disp_flt_cmd);
        strncpy(ErrMsg_SendTritium, DISP_NA_STR_LITERAL, ERR_CODE_LEN);
        memset(&ErrMsg_SendTritium, 0, sizeof(error_code_t));
    } else if (Error_ReadCarCAN != READCARCAN_ERR_NONE) {
        DisplayCmd_t rcc_flt_cmd = {
            .compOrCmd = (char*) DISPLAY_COMP_STR[DISP_FAULT_CODE], // "faulterr"
            .attr = "txt",
            .op = "=",
            .numArgs = 1,
            .argTypes = {STR_ARG},
            .args = {{.str = ErrMsg_ReadCarCAN}}
        };
        Display_Send(rcc_flt_cmd);
        strncpy(ErrMsg_ReadCarCAN, DISP_ERRMSG_NA, ERR_CODE_LEN);
        memset(&Error_ReadCarCAN, 0, sizeof(error_code_t));
    } else if (Error_IOState != IOSTATE_ERR_NONE) {
        DisplayCmd_t ios_flt_cmd = {
            .compOrCmd = (char*) DISPLAY_COMP_STR[DISP_FAULT_CODE], // "faulterr"
            .attr = "txt",
            .op = "=",
            .numArgs = 1,
            .argTypes = {STR_ARG},
            .args = {{.str = ErrMsg_IOState}}
        };
        Display_Send(ios_flt_cmd);
        strncpy(ErrMsg_IOState, DISP_ERRMSG_NA, ERR_CODE_LEN);
        memset(&Error_IOState, 0, sizeof(error_code_t));
    } else if (Error_UpdateDisplay != UPDATEDISPLAY_ERR_NONE) {
        DisplayCmd_t disp_flt_cmd = {
            .compOrCmd = (char*) DISPLAY_COMP_STR[DISP_FAULT_CODE], // "faulterr"
            .attr = "txt",
            .op = "=",
            .numArgs = 1,
            .argTypes = {STR_ARG},
            .args = {{.str = ErrMsg_UpdateDisplay}}
        };
        Display_Send(disp_flt_cmd);
        strncpy(ErrMsg_UpdateDisplay, DISP_ERRMSG_NA, ERR_CODE_LEN);
        memset(&Error_UpdateDisplay, 0, sizeof(error_code_t));
    } else {
        DisplayCmd_t no_flt_cmd = {
            .compOrCmd = (char*) DISPLAY_COMP_STR[DISP_FAULT_CODE], // "faulterr"
            .attr = "txt",
            .op = "=",
            .numArgs = 1,
            .argTypes = {STR_ARG},
            .args = {{.str = (char*)DISP_ERRMSG_NA}}
        };
        Display_Send(no_flt_cmd);
    }

    // Send SOC and SBPV values
    DisplayCmd_t soc_cmd = {
        .compOrCmd = (char*) DISPLAY_COMP_STR[DISP_SOC], // "soc"
        .attr = "val",
        .op = "=",
        .numArgs = 1,
        .argTypes = {INT_ARG},
        .args = {{.num = g_display_comp_vals[DISP_SOC]}}
    };
    Display_Send(soc_cmd);

    DisplayCmd_t supp_cmd = {
        .compOrCmd = (char*) DISPLAY_COMP_STR[DISP_SUPP_BATT], // "supp"
        .attr = "val",
        .op = "=",
        .numArgs = 1,
        .argTypes = {INT_ARG},
        .args = {{.num = g_display_comp_vals[DISP_SUPP_BATT]}}
    };
    Display_Send(supp_cmd);

    // Update pack current vars (value and sign)
    DisplayCmd_t packcurr_cmd = {
        .compOrCmd = (char*) DISPLAY_COMP_STR[DISP_PACK_CURRENT], // "pc"
        .attr = "val",
        .op = "=",
        .numArgs = 1,
        .argTypes = {INT_ARG},
        .args = {{.num = g_display_comp_vals[DISP_PACK_CURRENT]}}
    };
    Display_Send(packcurr_cmd);

    DisplayCmd_t packcurr_sign_cmd = {
        .compOrCmd = "vis",
        .attr = NULL,
        .op = NULL,
        .numArgs = 2,
        .argTypes = {STR_ARG, INT_ARG},
        .args = {
            {.str = (char*) DISPLAY_COMP_STR[DISP_PACK_CURR_SIGN]},  // "cs"
            {.num = g_display_comp_vals[DISP_PACK_CURR_SIGN]}
        }
    };
    Display_Send(packcurr_sign_cmd);

    return DISPLAY_ERR_NONE; // Can't do anything if this errors out
}

/**
 * @brief Displays the evacuation screen on the display
 * @param SOC_percent state of charge in percent
 * @param supp_mv supplemental battery pack voltage
 * @returns DisplayError_t
 * 
 *  [DO NOT USE THIS ONE. IT IS DEPRECATED IN FAVOR OF JUST USING Display_Error()]
 * 
 * @deprecated
 */
DisplayError_t Display_Evac(uint8_t SOC_percent, uint32_t supp_mv) {
    // Terminates any in progress command
    BSP_UART_Write(DISPLAY, (char*) TERMINATOR, strlen(TERMINATOR));

    // Switch to evac page
    Display_SetPage(EVAC);

    // Send SOC and SBPV values
    DisplayCmd_t soc_cmd = {
        .compOrCmd = (char*) DISPLAY_COMP_STR[DISP_SOC], // "soc"
        .attr = "val",
        .op = "=",
        .numArgs = 1,
        .argTypes = {INT_ARG},
        {
            {.num = SOC_percent}
        }
    };
    Display_Send(soc_cmd);

    DisplayCmd_t supp_cmd = {
        .compOrCmd = (char*) DISPLAY_COMP_STR[DISP_SUPP_BATT], // "supp"
        .attr = "val",
        .op = "=",
        .numArgs = 1,
        .argTypes = {INT_ARG},
        {
            {.num = supp_mv}
        }
    };
    Display_Send(supp_cmd);

    DisplayCmd_t packcurr_cmd = {
        .compOrCmd = (char*) DISPLAY_COMP_STR[DISP_PACK_CURRENT], // "pc"
        .attr = "val",
        .op = "=",
        .numArgs = 1,
        .argTypes = {INT_ARG},
        {
            {.num = g_display_comp_vals[DISP_PACK_CURRENT]}
        }
    };
    Display_Send(packcurr_cmd);

    DisplayCmd_t packcurr_sign_cmd = {
        .compOrCmd = "vis",
        .attr = NULL,
        .op = NULL,
        .numArgs = 2,
        .argTypes = {STR_ARG, INT_ARG},
        { 
            {.str = (char*) DISPLAY_COMP_STR[DISP_PACK_CURR_SIGN]}, // "cs"
            {.num = g_display_comp_vals[DISP_PACK_CURR_SIGN]}
        } 
    };
    Display_Send(packcurr_sign_cmd);

    return DISPLAY_ERR_NONE;
}
