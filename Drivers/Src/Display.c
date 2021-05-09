#include "bsp.h"
#include "Display.h"

/**
 * Convert signed 32-bit integer to string
 */
static char *to_charp(int32_t num, char *dest) {
    static char buf[12];
    bool neg;
    if (num < 0) {
        neg = true;
        num = -num;
    }
    int index = 11;
    buf[index] = '\0';
    do {
        buf[--index] = num % 10;
        num /= 10;
    } while (num > 9);
    if (neg) buf[--index] = '-';
    strcpy(dest, buf+index);
}

/**
 * Sets an object's attribute to a value
 * To send set text object 0's foreground color to green,
 * Run updateValue("t0", "pco", 2016);
 */
static ErrorStatus updateValue(char *obj, char *attr, int32_t val) {
    char buf[256];
    int len1 = strlen(obj);
    int len2 = strlen(attr);
    // construct the string "obj.attr=val"
    strcpy(buf, obj);
    buf[len1] = '.';
    strcpy(buf+len1+1, attr);
    buf[buf+len1+1+len2] = '=';
    to_charp(val, buf+len1+len2+2);
}

/*
 * Initialize the Nextion display
 */
void Display_Init() {
    BSP_UART_Init(UART_2);
    // The display sends 0x88 when ready, but that might be
    // before we initialize our UART
}
