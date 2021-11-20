#include "bsp.h"
#include "Display.h"
#include <math.h>
#include <string.h>

#define NEXTION_INSTRUCTION_SUCCESSFUL 0x01ffffff

static const char *DELIMITER = ".";
static const char *ASSIGNMENT = "=";
static const char *TERMINATOR = "\xff\xff\xff";

// Color defintions for display
static const uint16_t NEXTION_GREEN = 2016;
static const uint16_t NEXTION_RED = 63488;
static const uint16_t NEXTION_LIGHT_GREY = 42260;
static const uint16_t NEXTION_DARK_GREY = 23275;
static const uint16_t NEXTION_BURNT_ORANGE = 51872;

static inline IsNextionFailure(uint32_t val) {
    return ((val & ~0x00FFFFFF) != (1 << 24));
}


// List of names of possible strings to build a command out of
static enum CommandString_t {
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
 * Convert signed 32-bit integer to string
 * Returns pointer to the string
 */
static int *to_charp(int32_t num, char *dest) {
    bool neg = false;
    if (num < 0) {
        neg = true;
        num = -num;
    }
    int index = 12;
    dest[--index] = '\0';
    do {
        dest[--index] = (num % 10) + '0';
        num /= 10;
    } while (num > 9);
    if (num != 0) dest[--index] = num + '0';
    if (neg) dest[--index] = '-';
    return 12 - index; // return string length
}

/**
 * Sets an object's attribute to a value
 */
static ErrorStatus updateValue(enum CommandString_t obj_index, enum CommandString_t attr_index, int32_t val) {
    char buf[12]; // To store converted int
    char *number = to_charp(val, buf); // Convert integer to string
    char *obj = CommandStrings[obj_index];
    char *attr = CommandStrings[attr_index];
    int len1 = strlen(obj);
    int len2 = strlen(attr);

    // If not modifying a global, send obj
    if (len1 != 0) {
        BSP_UART_Write(UART_2, obj, len1);
        BSP_UART_Write(UART_2, DELIMITER, 1);
    }

    BSP_UART_Write(UART_2, attr, len2); // Send the attribute
    BSP_UART_Write(UART_2, ASSIGNMENT, 1);
    BSP_UART_Write(UART_2, number, strlen(number)); // Send the value
    BSP_UART_Write(UART_2, TERMINATOR, strlen(TERMINATOR));

    BSP_UART_Read(UART_2, buf);
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
    int32_t vel_fix = (uint32_t) floorf(vel * 10.0f);
    return updateValue(VELOCITY, VALUE, vel_fix);
}

/**
 * Update the cruise enable light on the display based on the given state
 */
ErrorStatus Display_CruiseEnable(State on) {
    if (on == ON) {
        return updateValue(CRUISE_ENABLE, PCO, NEXTION_GREEN);
    } else {
        return updateValue(CRUISE_ENABLE, PCO, NEXTION_LIGHT_GREY);
    }
}

/**
 * Update the cruise set light on the display based on the given state
 */
ErrorStatus Display_CruiseSet(State on) {
    if (on == ON) {
        return updateValue(CRUISE_SET, PCO, NEXTION_GREEN);
    } else {
        return updateValue(CRUISE_SET, PCO, NEXTION_LIGHT_GREY);
    }
}

/**
 * Set ERROR<idx> to err
 * If err is an empty string, then the error will be cleared
 */
ErrorStatus Display_SetError(int idx, char *err) {
    if (idx < 0 || idx > 5) return ERROR; // Index out of bounds
    return updateValue(ERROR0 + idx, TEXT, 0);
}


/**
 * Set the display to the main view
 */
ErrorStatus Display_SetMainView(void) {
    return updateValue(SYSTEM, PAGE, 1);
}

/**
 * Set the display back to the precharge view
 */
ErrorStatus Display_SetPrechargeView(void) {
    return updateValue(SYSTEM, PAGE, 0);
}
