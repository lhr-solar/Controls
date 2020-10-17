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
    char msg[GECKO_DATA_LENGTH];
    int flags = (status->cruiseEnabled << 3) |
        (status->cruiseSet << 2) |
        (status->regenEnabled << 1) |
        status->canError;

    #ifdef DEBUG
    assert(sizeof(float) == 4); // Just a sanity check
    #endif

    static union fi {float f; int i} data; // So we can interpret the float as a bitvector
    data.f = status->speed;
    for (int j=0; j<4; j++) {
        encodeByte(data.i & 0xFF, msg+j*2);
        data.i >>= 8; // shift the data down by a byte
    }
    
    encodeByte((uint8_t) flags, msg+4*2);


    // Send the bit vector of info to the Gecko controller
    BSP_UART_Write(UART_2, msg, GECKO_DATA_LENGTH);
}