#include "bsp.h"
#include "Display.h"
#include <math.h>
#include <string.h>

#define NEXTION_INSTRUCTION_SUCCESSFUL 0x01ffffff
// The conversion factor between meters per second to deci-miles per hour (3.6 / 1.609 * 10)
#define MPS_TO_dMPH 22.374f

static const char *DELIMITER = ".";
static const char *ASSIGNMENT = "=";
static const char *TERMINATOR = "\xff\xff\xff";
static const char *NO_ERROR = "No Error";

// Color defintions for display
static const uint16_t NEXTION_GREEN = 2016;
static const uint16_t NEXTION_RED = 63488;
static const uint16_t NEXTION_LIGHT_GREY = 42260;
static const uint16_t NEXTION_DARK_GREY = 23275;
static const uint16_t NEXTION_BURNT_ORANGE = 51872;

static inline int IsNextionFailure(uint32_t val) {
    return ((val & ~0x00FFFFFF) != (1 << 24));
}


// List of names of possible strings to build a command out of
enum CommandString_t {
    VELOCITY,
    CRUISE_ENABLE,
    CRUISE_SET,
    REGEN_ENABLE,
    VALUE,
    TEXT,
    PCO,
    SYSTEM,
    PAGE,
    ERROR0,
    ERROR1,
    ERROR2,
    ERROR3,
    ERROR4,
    ERROR5
};

// The command strings themselves
static char *CommandStrings[] = {
    "x0",
    "t1",
    "t2",
    "t3",
    "val",
    "txt",
    "pco",
    "",
    "page",
    "t4",
    "t5",
    "t6",
    "t7",
    "t8",
    "t9"
};

/**
 * Sends a string of the form "obj_index.attr_index=" or "attr_index=" over UART
 * Do not call on its own, should only be called by the updateValue subroutines
 */
static void sendStartOfAssignment(enum CommandString_t obj_index, enum CommandString_t attr_index) {
    char *obj = CommandStrings[obj_index];
    char *attr = CommandStrings[attr_index];
    int len = strlen(obj);

    if (len != 0) { // If not global
        BSP_UART_Write(UART_3, obj, len);
        BSP_UART_Write(UART_3, (char *) DELIMITER, strlen(DELIMITER));
    }

    BSP_UART_Write(UART_3, attr, strlen(attr)); // Send the attribute
    BSP_UART_Write(UART_3, (char *) ASSIGNMENT, 1);

}

/**
 * Sends out a string of the form "obj.attr=msg" or "attr=msg"
 * Use to update string fields of display objects
 */
static ErrorStatus updateStringValue(enum CommandString_t obj_index, enum CommandString_t attr_index, char *msg) {
    sendStartOfAssignment(obj_index, attr_index);

    BSP_UART_Write(UART_3, msg, strlen(msg));
    BSP_UART_Write(UART_3, (char *) TERMINATOR, strlen(TERMINATOR));

    // Get a response from the display
    uint8_t buf[8];
    BSP_UART_Read(UART_3, buf);
    int ret = *((uint32_t *) buf);
    return (IsNextionFailure(ret)) ? ERROR : SUCCESS;
}

/**
 * Sends out a string of the form "obj.attr=val" or "attr=val"
 * Use to update integer fields of display objects
 */
static ErrorStatus updateIntValue(enum CommandString_t obj_index, enum CommandString_t attr_index, int32_t val) {
    sendStartOfAssignment(obj_index, attr_index);

    char number[12]; // To store converted int
    sprintf(number, "%ld", val);

    BSP_UART_Write(UART_3, number, strlen(number));
    BSP_UART_Write(UART_3, (char *) TERMINATOR, strlen(TERMINATOR));

    // Get a response from the display
    uint8_t buf[8];
    BSP_UART_Read(UART_3, buf);
    int ret = *((uint32_t *) buf);
    return (IsNextionFailure(ret)) ? ERROR : SUCCESS;
}

/*
 * Initialize the Nextion display
 */
void Display_Init() {
    BSP_UART_Init(UART_3);
    // The display sends 0x88 when ready, but that might be
    // before we initialize our UART
}

/**
 * Set the displayed velocity to vel 
 */
ErrorStatus Display_SetVelocity(float vel) {
    int32_t vel_fix = (uint32_t) floorf(vel * MPS_TO_dMPH);
    return updateIntValue(VELOCITY, VALUE, vel_fix);
}

/**
 * Update the cruise enable light on the display based on the given state
 */
ErrorStatus Display_CruiseEnable(State on) {
    if (on == ON) {
        return updateIntValue(CRUISE_ENABLE, PCO, NEXTION_GREEN);
    } else {
        return updateIntValue(CRUISE_ENABLE, PCO, NEXTION_LIGHT_GREY);
    }
}

/**
 * Update the cruise set light on the display based on the given state
 */
ErrorStatus Display_CruiseSet(State on) {
    if (on == ON) {
        return updateIntValue(CRUISE_SET, PCO, NEXTION_GREEN);
    } else {
        return updateIntValue(CRUISE_SET, PCO, NEXTION_LIGHT_GREY);
    }
}

/**
 * Set ERROR<idx> to err
 * If err is an empty string, then the error will be cleared
 */
ErrorStatus Display_SetError(int idx, char *err) {
    if (idx < 0 || idx > 5) return ERROR; // Index out of bounds
    ErrorStatus err1 = updateStringValue(ERROR0 + idx, TEXT, err);
    ErrorStatus err2 = updateIntValue(ERROR0 + idx, PCO, NEXTION_GREEN);
    // If either is error, then had an error
    return (err1 == ERROR)  || (err2 == ERROR) ? ERROR : SUCCESS;
}

/**
 * Set the first error slot to say "No Error" and set its color to green
 * User must clear the remaining slots manually using Display_SetError
 */
ErrorStatus Display_NoErrors(void) {
    ErrorStatus err1 = updateIntValue(ERROR0, PCO, NEXTION_GREEN);
    ErrorStatus err2 = updateStringValue(ERROR0, TEXT, (char *) NO_ERROR);
    return err1 && err2; // If either one is error, then we had an error
}


/**
 * Set the display to the main view
 */
ErrorStatus Display_SetMainView(void) {
    return updateIntValue(SYSTEM, PAGE, 1);
}

/**
 * Set the display back to the precharge view
 */
ErrorStatus Display_SetPrechargeView(void) {
    return updateIntValue(SYSTEM, PAGE, 0);
}
