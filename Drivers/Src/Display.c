#include "bsp.h"
#include "Display.h"
#include <math.h>
#include <string.h>

#define NEXTION_INSTRUCTION_SUCCESSFUL 0xffffff01

// List of names of possible strings to build a command out of
static enum CommandString_t {
    VELOCITY,
    CRUISE_ENABLE,
    CRUISE_SET,
    REGEN_ENABLE,
    CAN_ERROR,
    VALUE,
    TEXT,
    SYSTEM,
    PAGE
};

// The command strings themselves
static char *CommandStrings[] = {
    "x0",
    "t1",
    "t2",
    "t3",
    "t4",
    "val",
    "txt",
    "",
    "page"
};

/**
 * Convert signed 32-bit integer to string
 */
static char *to_charp(int32_t num, char *dest) {
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
    to_charp(val, buf+len1+len2+2);

    // printf("Command is \"%s\"\n", (len1 == 0) ? buf+1 : buf);

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
    BSP_UART_Init(UART_2);
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
