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

#include "Tasks.h"  // for os and fault error codes
#include "bsp.h"    // for writing to UART

#define PAGE_STR_SIZE 7
#define FAULT_STR_SIZE 20
#define SOC_STR_SIZE 13
#define SUPP_STR_SIZE 18

#define DISP_OUT kUart3
#define MAX_MSG_LEN 32
#define MAX_ARG_LEN 16
// Assignment commands have only 1 arg, an operator, and an attribute
#define IS_ASSIGN_CMD(cmd)                                 \
    ((((cmd).comp_or_cmd != NULL) && ((cmd).op != NULL) && \
      ((cmd).attr != NULL) && ((cmd).num_args == 1)))
// Operational commands have no attribute and no operator, just a command and >=
// 0 arguments
#define IS_OP_CMD(cmd) ((cmd).op == NULL && (cmd).attr == NULL)

static const char *terminator = "\xff\xff\xff";

DisplayError DisplayInit() {
    BspUartInit(DISP_OUT);
    return DisplayReset();
}

DisplayError DisplaySend(DisplayCmd cmd) {
    char msg_args[MAX_MSG_LEN];
    if (IS_ASSIGN_CMD(cmd)) {
        if (cmd.arg_types[0] == kIntArg) {
            sprintf(msg_args, "%d", (int)cmd.args[0].num);
        } else {
            if (cmd.args[0].str == NULL) {
                return kDisplayErrParse;
            }
            sprintf(msg_args, "%s", cmd.args[0].str);
        }

        BspUartWrite(DISP_OUT, cmd.comp_or_cmd, strlen(cmd.comp_or_cmd));
        BspUartWrite(DISP_OUT, ".", 1);
        BspUartWrite(DISP_OUT, cmd.attr, strlen(cmd.attr));
        BspUartWrite(DISP_OUT, cmd.op, strlen(cmd.op));
    } else if (IS_OP_CMD(cmd)) {
        msg_args[0] = ' ';  // No args
        msg_args[1] = '\0';
        if (cmd.num_args > DISPLAY_CMD_MAX_ARGS) {
            return kDisplayErrOther;
        }
        if (cmd.num_args >= 1) {  // If there are arguments
            for (int i = 0; i < cmd.num_args; i++) {
                char arg[MAX_ARG_LEN];
                if (cmd.arg_types[i] == kIntArg) {
                    sprintf(arg, "%d", (int)cmd.args[i].num);
                } else {
                    sprintf(arg, "%s", cmd.args[i].str);
                }

                strcat(msg_args, arg);

                if (i < cmd.num_args - 1) {  // delimiter
                    strcat(msg_args, ",");
                }
            }
        }
        BspUartWrite(DISP_OUT, cmd.comp_or_cmd, strlen(cmd.comp_or_cmd));
    } else {  // Error parsing command struct
        return kDisplayErrParse;
    }

    if (cmd.num_args >= 1) {  // If there are arguments
        BspUartWrite(DISP_OUT, msg_args, strlen(msg_args));
    }

    BspUartWrite(DISP_OUT, (char *)terminator, strlen(terminator));

    return kDisplayErrNone;
}

DisplayError DisplayReset() {
    DisplayCmd rest_cmd = {
        .comp_or_cmd = "rest", .attr = NULL, .op = NULL, .num_args = 0};

    BspUartWrite(DISP_OUT, (char *)terminator,
                 strlen(terminator));  // Terminates any in progress command

    return DisplaySend(rest_cmd);
}

DisplayError DisplayFault(ErrorCode fault_code) {
    BspUartWrite(DISP_OUT, (char *)terminator,
                 strlen(terminator));  // Terminates any in progress command

    char fault_page[PAGE_STR_SIZE] = "page 2";
    BspUartWrite(DISP_OUT, fault_page, strlen(fault_page));
    BspUartWrite(DISP_OUT, (char *)terminator, strlen(terminator));

    char set_fault_code[FAULT_STR_SIZE];
    sprintf(set_fault_code, "%s\"%04x\"",
            "faulterr.txt=", (uint16_t)fault_code);
    BspUartWrite(DISP_OUT, set_fault_code, strlen(set_fault_code));
    BspUartWrite(DISP_OUT, (char *)terminator, strlen(terminator));

    return kDisplayErrNone;
}

DisplayError DisplayEvac(uint8_t soc_percent, uint32_t supp_mv) {
    BspUartWrite(DISP_OUT, (char *)terminator,
                 strlen(terminator));  // Terminates any in progress command

    char evac_page[PAGE_STR_SIZE] = "page 3";
    BspUartWrite(DISP_OUT, evac_page, strlen(evac_page));
    BspUartWrite(DISP_OUT, (char *)terminator, strlen(terminator));

    char soc[SOC_STR_SIZE];
    sprintf(soc, "%s%d", "soc.val=", (int)soc_percent);
    BspUartWrite(DISP_OUT, soc, strlen(soc));
    BspUartWrite(DISP_OUT, (char *)terminator, strlen(terminator));

    char supp[SUPP_STR_SIZE];
    sprintf(supp, "%s%d", "supp.val=", (int)supp_mv);
    BspUartWrite(DISP_OUT, supp, strlen(supp));
    BspUartWrite(DISP_OUT, (char *)terminator, strlen(terminator));

    return kDisplayErrNone;
}
