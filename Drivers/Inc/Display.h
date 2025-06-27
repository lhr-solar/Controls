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

#ifndef __DISPLAY_H
#define __DISPLAY_H

#include "common.h"
#include "Tasks.h"

// #define DISP_OUT DISPLAY
#define MAX_MSG_LEN 32
#define MAX_ARG_LEN 17

#define MAX_ARGS    2 // maximum # of arguments in a command packet


#define FOREACH_DISPLAY_COMPONENT(DISP_COMP) \
    /* Boolean components */                 \
    DISP_COMP(DISP_HEARTBEAT,        "hb")   \
    DISP_COMP(DISP_PACK_CURR_SIGN,   "cs")   \
    DISP_COMP(DISP_MC_CURR_SIGN,     "mcs")  \
    DISP_COMP(DISP_BRAKE,            "brake")\
    DISP_COMP(DISP_RIGHT_BLINK,      "rightblink")\
    DISP_COMP(DISP_LEFT_BLINK,       "leftblink")\
    /* Contactor-ish (still bool, but logically different) */ \
    DISP_COMP(DISP_ARRAY_EN,         "arren")\
    DISP_COMP(DISP_ARRAY_PC,         "arrpc")\
    DISP_COMP(DISP_MOTOR_EN,         "moten")\
    DISP_COMP(DISP_MOTOR_PC,         "motpc")\
    /* Non-boolean components */             \
    DISP_COMP(DISP_VELOCITY,         "vel")  \
    DISP_COMP(DISP_ACCEL_METER,      "accel")\
    DISP_COMP(DISP_SOC,              "soc")  \
    DISP_COMP(DISP_SUPP_BATT,        "supp") \
    DISP_COMP(DISP_CRUISE_ST,        "cruiseSt") \
    DISP_COMP(DISP_REGEN_ST,         "rbsSt")\
    DISP_COMP(DISP_PACK_VOLTAGE,     "pv")   \
    DISP_COMP(DISP_PACK_CURRENT,     "pc")   \
    DISP_COMP(DISP_PACK_TEMP,        "pt")   \
    DISP_COMP(DISP_MC_BUS_VOLTAGE,   "mcv")  \
    DISP_COMP(DISP_MC_BUS_CURRENT,   "mcc")  \
    DISP_COMP(DISP_HEAT_SINK_TEMP,   "heatsink")\
    DISP_COMP(DISP_GEAR,             "gear") \
    DISP_COMP(DISP_MOT_LIMIT,        "motorlimit") \
    /* Fault components */                   \
    DISP_COMP(DISP_OS_CODE,          "oserr")\
    DISP_COMP(DISP_FAULT_CODE,       "faulterr")\
    DISP_COMP(DISP_EVAC_MSG,         "evac") \
    DISP_COMP(DISP_EVAC_BPS_FAULT,   "bpsfaulterr")

/**
 * Enum and corresponding array for easy component selection.
 */
typedef enum {
    #define GENERATE_DISPLAY_ENUM(name, str) name,
    FOREACH_DISPLAY_COMPONENT(GENERATE_DISPLAY_ENUM)
    DISP_NUM_COMPONENTS
} Component_t;

/**
 * Values corresponding to the component enum.
 */
extern uint32_t g_display_comp_vals[DISP_NUM_COMPONENTS];
extern const char *DISPLAY_COMP_STR[DISP_NUM_COMPONENTS];

/**
 * Error types
 */
typedef enum {                  // Currently only ERR_NONE and ERR_PARSE are used
    DISPLAY_ERR_NONE = 0,       // No Error
    DISPLAY_ERR_PARSE = 1 << 0, // Error parsing command struct passed to Display_Send
    DISPLAY_ERR_OTHER = 1 << 1, // Other nextion display error

    // All unused
    // DISPLAY_ERR_INV_INSTR	= 1 << 2,	// Invalid instruction passed to nextion (0x00)
    // DISPLAY_ERR_INV_COMP	= 1 << 3,	// Invalid component id passed to nextion (0x02)
    // DISPLAY_ERR_INV_PGID 	= 1 << 4,	// Invalid page id passed to nextion	(0x03)
    // DISPLAY_ERR_INV_VAR		= 1 << 5,	// Invalid variable name passed to nextion	(0x1A)
    // DISPLAY_ERR_INV_VAROP	= 1 << 6,	// Invalid variable operation passed to nextion	(0x1B)
    // DISPLAY_ERR_ASSIGN		= 1 << 7,	// Assignment failure nextion	(0x1C)
    // DISPLAY_ERR_PARAMS		= 1 << 8,	// Invalid number of parameters passed to nextion
    // (0x1E)
    // DISPLAY_ERR_MAX_ARGS	= 1 << 9    // Command arg list exceeded MAX_ARGS elements
} DisplayError_t;

/**
 * All three pages on the HMI
 */
typedef enum Page {
    STARTUP = (uint32_t)0,
    INFO = (uint32_t)1,
    FAULT = (uint32_t)2,
    EVAC = (uint32_t)3
} Page_t;

/**
 * Argument types
 */
typedef enum { STR_ARG, INT_ARG } Arg_e;

/**
 * Packages relevant display command data
 */
typedef struct {
    char *compOrCmd;
    char *attr;
    char *op;
    uint8_t numArgs;
    Arg_e argTypes[MAX_ARGS];
    union {
        char *str;
        uint32_t num;
    } args[MAX_ARGS];
} DisplayCmd_t;

/**
 * @brief Sends a display message.
 * @returns DisplayError_t
 */
DisplayError_t Display_Send(DisplayCmd_t cmd);

/**
 * @brief Initializes the display
 * @returns DisplayError_t
 */
DisplayError_t Display_Init(void);

/**
 * @brief Resets (reboots) the display
 * @returns DisplayError_t
 */
DisplayError_t Display_Reset(void);

/**
 * @brief Overwrites any processing commands and triggers the display fault screen
 * @param app_err_str the app error string message to display. MUST be length < 16. Pass in NULL to
 * display "N/A"
 * @param os_err_str the os error string message to display. MUST be length < 16. Pass in NULL to
 * display "N/A"
 * @param is_evac_needed whether evac is required.
 * @returns DisplayError_t
 */
DisplayError_t Display_Error(const char *app_err_str, const char *os_err_str, bool is_evac_needed, BPSFaultErr_e bps_err);

/**
 * 
 */

/**
 * @brief Overwrites any processing commands and triggers the evacuation screen
 * @param SOC_percent the state of charge of the battery in percent
 * @param supp_mv the voltage of the battery in millivolts
 * @returns DisplayError_t
 */
DisplayError_t Display_Evac(uint8_t SOC_percent, uint32_t supp_mv);

/**
 * @brief Changes the page of the display
 * @returns DisplayError_t
 */
DisplayError_t Display_SetPage(Page_t page);

DisplayError_t Display_Refresh(void);

#endif

/* @} */
