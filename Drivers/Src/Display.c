#include "bsp.h"
#include "Display.h"

#define GECKO_DATA_LENGTH 5

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

    // The update gecko will expect five bytes: a float a some flags
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
    memcpy(msg, &(status->speed), sizeof(float));
    msg[GECKO_DATA_LENGTH-1] = flags;


    // Send the bit vector of info to the Gecko controller
    BSP_UART_Write(UART_2, msg, GECKO_DATA_LENGTH);
}