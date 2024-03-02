/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file Display.h
 * @brief Function prototypes for the display driver.
 *
 * This contains function prototypes relevant to sending/receiving messages
 * to/from our Nextion HMI.
 *
 * @defgroup Display
 * @addtogroup Display
 * @{
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include "Tasks.h"   // for os and fault error locs
#include "common.h"  // common headers

#define DISPLAY_CMD_MAX_ARGS 2  // maximum # of arguments in a command packet

/**
 * Error types
 */
typedef enum {  // Currently only ERR_NONE and ERR_PARSE are used
    kDisplayErrNone,
    kDisplayErrParse,     // Error parsing command struct passed to Display_Send
    kDisplayErrInvInstr,  // Invalid instruction passed to nextion (0x00)
    kDisplayErrInvComp,   // Invalid component id passed to nextion (0x02)
    kDisplayErrInvPgid,   // Invalid page id passed to nextion	(0x03)
    kDisplayErrInvVar,    // Invalid variable name passed to nextion	(0x1A)
    kDisplayErrInvVarop,  // Invalid variable operation passed to nextion
                          // (0x1B)
    kDisplayErrAssign,    // Assignment failure nextion	(0x1C)
    kDisplayErrParams,    // Invalid number of parameters passed to nextion
                          // (0x1E)
    kDisplayErrMaxArgs,   // Command arg list exceeded DISPLAY_CMD_MAX_ARGS
                          // elements
    kDisplayErrOther      // Other nextion display error
} DisplayError;

/**
 * All three pages on the HMI
 */
typedef enum { kStartup = 0, kInfo, kFault } Page;

/**
 * Argument types
 */
typedef enum { kStrArg, kIntArg } Arg;

/**
 * Packages relevant display command data
 */
typedef struct {
    char* comp_or_cmd;
    char* attr;
    char* op;
    uint8_t num_args;
    Arg arg_types[DISPLAY_CMD_MAX_ARGS];  // TRUE for integers, FALSE for
                                          // strings
    union {
        char* str;
        uint32_t num;
    } args[DISPLAY_CMD_MAX_ARGS];
} DisplayCmd;

/**
 * @brief Sends a display message.
 * @returns DisplayError_t
 */
DisplayError DisplaySend(DisplayCmd cmd);

/**
 * @brief Initializes the display
 * @returns DisplayError_t
 */
DisplayError DisplayInit(void);

/**
 * @brief Resets (reboots) the display
 * @returns DisplayError_t
 */
DisplayError DisplayReset(void);

/**
 * @brief Overwrites any processing commands and triggers the display fault
 * screen
 * @param faultCode the application's fault code (will be displayed in hex)
 * @returns DisplayError_t
 */
DisplayError DisplayFault(ErrorCode fault_code);

/**
 * @brief Overwrites any processing commands and triggers the evacuation screen
 * @param SOC_percent the state of charge of the battery in percent
 * @param supp_mv the voltage of the battery in millivolts
 * @returns DisplayError_t
 */
DisplayError DisplayEvac(uint8_t soc_percent, uint32_t supp_mv);

#endif

/* @} */
