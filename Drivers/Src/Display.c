#include "bsp.h"
#include "Display.h"
#include <math.h>
#include <string.h>

#define NEXTION_INSTRUCTION_SUCCESSFUL 0xffffff01

static inline NextionIsFailure(uint32_t val) {
    return ((val & 0xFFFFFF00) == 0xFFFFFF00 && (val & 0xFF) != 1);
}


// List of names of possible strings to build a command out of
static enum CommandString_t {
    VELOCITY,
    CRUISE_ENABLE,
    CRUISE_SET,
    REGEN_ENABLE,
    VALUE,
    TEXT,
    SYSTEM,
    PAGE,
    ERROR_NONE,
    ERROR1,
    ERROR2,
    ERROR3,
};

// The command strings themselves
static char *CommandStrings[] = {
    "x0",
    "t1",
    "t2",
    "t3",
    "val",
    "txt",
    "",
    "page",
    "t5",
    "t6",
    "t7",
    "t8"

};

/**
 * Convert signed 32-bit integer to string
 * Returns the length of the string
 */
static int *to_charp(int32_t num, char *dest) {
    static char buf[12];
    bool neg = false;
    if (num < 0) {
        neg = true;
        num = -num;
    }
    int index = 12;
    buf[--index] = '\0';
    do {
        buf[--index] = (num % 10) + '0';
        num /= 10;
    } while (num > 9);
    if (num != 0) buf[--index] = num + '0';
    if (neg) buf[--index] = '-';
    strcpy(dest, buf+index);
    return 12 - index; // return string length
}

/**
 * Sets an object's attribute to a value
 */
static ErrorStatus updateValue(enum CommandString_t obj_index, enum CommandString_t attr_index, int32_t val) {
    char buf[256];
    char *obj = CommandStrings[obj_index];
    char *attr = CommandStrings[attr_index];
    int len1 = strlen(obj);
    int len2 = strlen(attr);

    // construct the string "obj.attr=val"
    strcpy(buf, obj);
    buf[len1] = '.';
    strcpy(buf+len1+1, attr);
    buf[len1+1+len2] = '=';
    int len3 = to_charp(val, buf+len1+len2+2);
    buf[len1+len2+len3+1] = 0xFF; // Terminate command with 0xFFFFFF
    buf[len1+len2+len3+2] = 0xFF;
    buf[len1+len2+len3+3] = 0xFF;
    buf[len1+len2+len3+4] = '\0';

    printf("Command is \"%s\"\n", (len1 == 0) ? buf+1 : buf);

    // Get rid of extraneous period in global attribute
    BSP_UART_Write(UART_2, (len1 == 0) ? buf+1 : buf, strlen(buf));

    BSP_UART_Read(UART_2, buf);
    int ret = *((uint32_t *) buf);
    return (ret == NEXTION_INSTRUCTION_SUCCESSFUL) ? SUCCESS : ERROR;
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
    int32_t vel_fix = (uint32_t) floor(vel * 10.0f);
    return updateValue(VELOCITY, VALUE, vel_fix);
}


/**
 * Set the display to the main view
 */
ErrorStatus Display_SetMainView(void) {
    return updateValue(SYSTEM, PAGE, 1);
}
