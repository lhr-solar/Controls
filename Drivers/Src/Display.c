#include "bsp.h"
#include "Display.h"

#define GECKO_DATA_LENGTH 5*2

/*
 * Encodes a byte into two hex digits and places them at buf
 * @param byte the given data byte
 * @param buf the output buffer
 * @return void
 */
static void encodeByte(uint8_t byte, uint8_t *buf) {
    uint8_t hi = (byte >> 4) & 0x0F;
    uint8_t lo = byte & 0x0F;
    buf[0] = hi < 10 ? (hi + '0') : (hi - 10 + 'A');
    buf[1] = lo < 10 ? (lo + '0') : (lo - 10 + 'A');
    // e.g encoding the byte `42` will write the characters
    // '2' and 'A' to the locations pointed at by buf and buf+1
}

/*
 * Initialize the Wonder Gecko communications.
 */
void Display_Init() {
    BSP_UART_Init(UART_2);
}

/*
 * Updates the Gecko display with the data provided.
 * @param status the new display data
 * @return void
 */
void Display_SetData(display_data_t *status) {

    // The update gecko will expect ten bytes: a float a some flags
    // Each byte will be encoded as two hex digits
    // The float will be sent in little endian format, and the flags
    // will be encoded in the order they're declared in the struct,
    // starting at bit 3 and going til bit 0 of the fifth byte
    uint8_t msg[GECKO_DATA_LENGTH];
    // Pack the flags into a byte
    int flags = (status->cruiseEnabled << 3) |
        (status->cruiseSet << 2) |
        (status->regenEnabled << 1) |
        status->canError;

    static union fi {float f; int i;} data; // So we can interpret the float as a bitvector
    data.f = status->speed;
    for (int j=0; j<4; j++) {
        encodeByte(data.i & 0xFF, msg+j*2); // enocde each byte of the float as two hex digits
        data.i >>= 8; // Get rid of the bit we just encoded and prepare the next one
    }

    // Say, for example, that the speed is 38.0
    // In IEEE-754 floating point format, it is represented as 0x42180000
    // At this point in the code, the msg array will have "00001842" as its first eight bytes
    
    encodeByte((uint8_t) flags, msg+4*2);

    // At this point, the last byte of the msg array represents the flags


    // Send the bit vector of info to the Gecko controller
    BSP_UART_Write(UART_2, (char *) msg, GECKO_DATA_LENGTH);
}