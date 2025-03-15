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

#include "ReadCarCAN.h"
#include "ReadTritium.h"
#include "UpdateDisplay.h"

// Assignment commands have only 1 arg, an operator, and an attribute
#define IS_ASSIGN_CMD(cmd)                              \
    (cmd.compOrCmd != NULL && cmd.op != NULL &&         \
     cmd.attr != NULL && cmd.numArgs == 1)
// Operational commands have no attribute and no operator, just a command and
// >= 0 arguments
#define IS_OP_CMD(cmd) (cmd.op == NULL && cmd.attr == NULL)

static const char *TERMINATOR = "\xff\xff\xff";

const char *DISPLAY_COMP_STR[NUM_COMPONENTS] = {
    // Boolean components
    "arr", "hb", "cs", "mcs", "brake", "mot",
    // Non-boolean components
    "vel", "accel", "soc", "supp", "cruiseSt", "rbsSt", "pv", "pc", "pt", "mcv",
    "mcc", "heatsink", "gear",
    // Fault code components
    "oserr", "faulterr"
};

extern uint32_t DISPLAY_COMP_VALS[NUM_COMPONENTS];


/**
 * @brief Initializes the display driver
 * @returns DisplayError_t
 */
DisplayError_t Display_Init() {
    BSP_UART_Init(DISP_OUT);
    return Display_Reset();
}

/**
 * @brief Sends a command to the display
 * @param cmd command to send
 * @returns DisplayError_t
 */
DisplayError_t Display_Send(DisplayCmd_t cmd) {
    char msgArgs[MAX_MSG_LEN];

    if (IS_ASSIGN_CMD(cmd)) {
        if (cmd.argTypes[0] == INT_ARG) {
            sprintf(msgArgs, "%d", (int) cmd.args[0].num);
        } else { // STR_ARG
            if (cmd.args[0].str == NULL)
            return DISPLAY_ERR_PARSE;
            sprintf(msgArgs, "%s", cmd.args[0].str);
        }

        BSP_UART_Write(DISP_OUT, cmd.compOrCmd, strlen(cmd.compOrCmd));
        BSP_UART_Write(DISP_OUT, ".", 1);
        BSP_UART_Write(DISP_OUT, cmd.attr, strlen(cmd.attr));
        BSP_UART_Write(DISP_OUT, cmd.op, strlen(cmd.op));
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

        BSP_UART_Write(DISP_OUT, cmd.compOrCmd, strlen(cmd.compOrCmd));
    } 
    else { // Error parsing command struct
        return DISPLAY_ERR_PARSE;
    }

    // If there are arguments, write them
    if (cmd.numArgs >= 1) { 
        BSP_UART_Write(DISP_OUT, msgArgs, strlen(msgArgs));
    }

    BSP_UART_Write(DISP_OUT, (char *)TERMINATOR, strlen(TERMINATOR));

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
    BSP_UART_Write(DISP_OUT, (char *)TERMINATOR, strlen(TERMINATOR));

    return Display_Send(restCmd);
}

/**
 * @brief Overwrites any processing commands and triggers the display fault screen
 * @returns DisplayError_t
 */
DisplayError_t Display_Error() {
    // Terminates any in progress command
    BSP_UART_Write(DISP_OUT, (char*) TERMINATOR, strlen(TERMINATOR)); 

    // Switch to fault page
    DisplayCmd_t err_pg_cmd = {
        .compOrCmd = "page",
        .attr = NULL,
        .op = NULL,
        .numArgs = 1,
        .argTypes = {INT_ARG},
        {
            {.num = FAULT}
        }
    };
    Display_Send(err_pg_cmd);

    // Display OS error if there is one
    DisplayCmd_t os_flt_cmd = {
        .compOrCmd = (char*) DISPLAY_COMP_STR[19], // "oserr"
        .attr = "txt",
        .op = "=",
        .numArgs = 1,
        .argTypes = {STR_ARG},
        {
            {.str = ErrMsg_OS}
        }
    };
    Display_Send(os_flt_cmd);
    strncpy(ErrMsg_OS, "\"N/A\"", 6);
    memset(&Error_OS, 0, sizeof(error_code_t));

    // Display other errors if there are any.
    // Prioritized errors in order of importance:
    // 1. ReadTritium
    // 2. ReadCarCAN
    // 3. UpdateDisplay
    if (Error_ReadTritium != T_NONE) {
        DisplayCmd_t moco_flt_cmd = {
            .compOrCmd = (char*) DISPLAY_COMP_STR[20], // "faulterr"
            .attr = "txt",
            .op = "=",
            .numArgs = 1,
            .argTypes = {STR_ARG},
            {
                {.str = ErrMsg_ReadTritium}
            }
        };
        Display_Send(moco_flt_cmd);
        strncpy(ErrMsg_ReadTritium, "\"N/A\"", 6);
        memset(&Error_ReadTritium, 0, sizeof(error_code_t));
    } else if (Error_ReadCarCAN != READCARCAN_ERR_NONE) {
        DisplayCmd_t rcc_flt_cmd = {
            .compOrCmd = (char*) DISPLAY_COMP_STR[20], // "faulterr"
            .attr = "txt",
            .op = "=",
            .numArgs = 1,
            .argTypes = {STR_ARG},
            {
                {.str = ErrMsg_ReadCarCAN}
            }
        };
        Display_Send(rcc_flt_cmd);
        strncpy(ErrMsg_ReadCarCAN, "\"N/A\"", 6);
        memset(&Error_ReadCarCAN, 0, sizeof(error_code_t));
    } else if (Error_UpdateDisplay != UPDATEDISPLAY_ERR_NONE) {
        DisplayCmd_t disp_flt_cmd = {
            .compOrCmd = (char*) DISPLAY_COMP_STR[20], // "faulterr"
            .attr = "txt",
            .op = "=",
            .numArgs = 1,
            .argTypes = {STR_ARG},
            {
                {.str = ErrMsg_UpdateDisplay}
            }
        };
        Display_Send(disp_flt_cmd);
        strncpy(ErrMsg_UpdateDisplay, "\"N/A\"", 6);
        memset(&Error_UpdateDisplay, 0, sizeof(error_code_t));
    } else {
        DisplayCmd_t no_flt_cmd = {
            .compOrCmd = (char*) DISPLAY_COMP_STR[20], // "faulterr"
            .attr = "txt",
            .op = "=",
            .numArgs = 1,
            .argTypes = {STR_ARG},
            {
                {.str = "\"N/A\""}
            }
        };
        Display_Send(no_flt_cmd);
    }

    // Update pack current vars (value and sign)
    DisplayCmd_t packcurr_cmd = {
        .compOrCmd = (char*) DISPLAY_COMP_STR[13], // "pc"
        .attr = "val",
        .op = "=",
        .numArgs = 1,
        .argTypes = {INT_ARG},
        {
            {.num = DISPLAY_COMP_VALS[13]}
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
            {.str = (char*) DISPLAY_COMP_STR[2]},  // "cs"
            {.num = DISPLAY_COMP_VALS[2]}
        }
    };
    Display_Send(packcurr_sign_cmd);

    return DISPLAY_ERR_NONE;
}

/**
 * @brief Displays the evacuation screen on the display
 * @param SOC_percent state of charge in percent
 * @param supp_mv supplemental battery pack voltage
 * @returns DisplayError_t
 */
DisplayError_t Display_Evac(uint8_t SOC_percent, uint32_t supp_mv) {
    // Terminates any in progress command
    BSP_UART_Write(DISP_OUT, (char*) TERMINATOR, strlen(TERMINATOR));

    // Switch to evac page
    DisplayCmd_t evac_pg_cmd = {
        .compOrCmd = "page",
        .attr = NULL,
        .op = NULL,
        .numArgs = 1,
        .argTypes = {INT_ARG},
        {
            {.num = EVAC}
        }
    };
    Display_Send(evac_pg_cmd);

    // Send SOC and SBPV values
    DisplayCmd_t soc_cmd = {
        .compOrCmd = (char*) DISPLAY_COMP_STR[8], // "soc"
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
        .compOrCmd = (char*) DISPLAY_COMP_STR[9], // "supp"
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
        .compOrCmd = (char*) DISPLAY_COMP_STR[13], // "pc"
        .attr = "val",
        .op = "=",
        .numArgs = 1,
        .argTypes = {INT_ARG},
        {
            {.num = DISPLAY_COMP_VALS[13]}
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
            {.str = (char*) DISPLAY_COMP_STR[2]}, // "cs"
            {.num = DISPLAY_COMP_VALS[2]}
        } 
    };
    Display_Send(packcurr_sign_cmd);

    return DISPLAY_ERR_NONE;
}
